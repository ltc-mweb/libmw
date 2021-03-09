#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/common/Macros.h>
#include <mw/mmr/Backend.h>
#include <mw/mmr/Node.h>
#include <mw/file/FilePath.h>
#include <mw/file/AppendOnlyFile.h>
#include <mw/db/LeafDB.h>
#include <mw/exceptions/NotFoundException.h>
#include <libmw/interfaces/db_interface.h>

MMR_NAMESPACE

// TODO: Add pruning support
class FileBackend : public IBackend
{
public:
    static std::shared_ptr<FileBackend> Open(
        const char dbPrefix,
        const FilePath& mmr_dir,
        const uint32_t file_index,
        const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper)
    {
        const FilePath path = GetPath(mmr_dir, dbPrefix, file_index);
        return std::make_shared<FileBackend>(
            dbPrefix,
            mmr_dir,
            AppendOnlyFile::Load(path),
            pDBWrapper
        );
    }

    FileBackend(
        const char dbPrefix,
        const FilePath& mmr_dir,
        const AppendOnlyFile::Ptr& pHashFile,
        const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper)
        : m_dbPrefix(dbPrefix), m_dir(mmr_dir), m_pHashFile(pHashFile), m_pDatabase(pDBWrapper) {}

    void AddLeaf(const Leaf& leaf) final
    {
        m_leafMap[leaf.GetHash()] = m_leaves.size();
        m_leaves.push_back(leaf);
        AddHash(leaf.GetHash());

        auto rightHash = leaf.GetHash();
        auto nextIdx = leaf.GetNodeIndex().GetNext();
        while (!nextIdx.IsLeaf())
        {
            const mw::Hash leftHash = GetHash(nextIdx.GetLeftChild());
            const Node node = Node::CreateParent(nextIdx, leftHash, rightHash);

            AddHash(node.GetHash());
            rightHash = node.GetHash();
            nextIdx = nextIdx.GetNext();
        }
    }

    void AddHash(const mw::Hash& hash) final { m_pHashFile->Append(hash.vec()); }

    void Rewind(const LeafIndex& nextLeafIndex) final
    {
        m_pHashFile->Rewind(nextLeafIndex.GetPosition() * 32);
    }

    uint64_t GetNumLeaves() const noexcept final
    {
        return Index::At(m_pHashFile->GetSize() / mw::Hash::size()).GetLeafIndex();
    }

    mw::Hash GetHash(const Index& idx) const final
    {
        return mw::Hash(m_pHashFile->Read(idx.GetPosition() * mw::Hash::size(), mw::Hash::size()));
    }

    Leaf GetLeaf(const LeafIndex& idx) const final
    {
        mw::Hash hash = GetHash(idx.GetNodeIndex());
        auto it = m_leafMap.find(hash);
        if (it != m_leafMap.end()) {
            return m_leaves[it->second];
        }

        LeafDB ldb(m_dbPrefix, m_pDatabase.get());
        auto pLeaf = ldb.Get(idx, std::move(hash));
        if (!pLeaf) {
            ThrowNotFound_F("Can't get leaf at position {} with hash {}", idx.GetPosition(), hash);
        }

        return std::move(*pLeaf);
    }

    void Commit(const uint32_t file_index, const std::unique_ptr<libmw::IDBBatch>& pBatch) final
    {
        m_pHashFile->Commit(GetPath(m_dir, m_dbPrefix, file_index));

        // Update database
        LeafDB(m_dbPrefix, m_pDatabase.get(), pBatch.get())
            .Add(m_leaves);

        m_leaves.clear();
        m_leafMap.clear();
    }

private:
    static FilePath GetPath(const FilePath& dir, const char prefix, const uint32_t file_index)
    {
        return dir.GetChild(StringUtil::Format("{}{:0>6}.dat", prefix, file_index));;
    }

    char m_dbPrefix;
    FilePath m_dir;
    AppendOnlyFile::Ptr m_pHashFile;
    std::vector<Leaf> m_leaves;
    std::map<mw::Hash, size_t> m_leafMap;
    std::shared_ptr<libmw::IDBWrapper> m_pDatabase;
};

END_NAMESPACE