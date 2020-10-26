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
#include <libmw/interfaces.h>

MMR_NAMESPACE

// TODO: Add pruning support
class FileBackend : public IBackend
{
public:
    static std::shared_ptr<FileBackend> Open(
        const FilePath& path,
        const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper)
    {
        return std::make_shared<FileBackend>(
            AppendOnlyFile::Load(path.GetChild("pmmr_hash.bin")),
            pDBWrapper
        );
    }

    FileBackend(
        const AppendOnlyFile::Ptr& pHashFile,
        const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper)
        : m_pHashFile(pHashFile), m_pDatabase(pDBWrapper) {}

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
        LeafDB ldb(m_pDatabase.get());
        auto pLeaf = ldb.Get(idx, std::move(hash));
        if (!pLeaf) {
            ThrowNotFound_F("Can't get leaf at position {} with hash {}", idx.GetPosition(), hash);
        }
        return std::move(*pLeaf);
    }

    void Commit(const std::unique_ptr<libmw::IDBBatch>& pBatch = nullptr) final
    {
        m_pHashFile->Commit();
        LeafDB ldb(m_pDatabase.get(), pBatch.get());
        ldb.Add(m_leaves);
        m_leaves.clear();
        m_leafMap.clear();
    }

    void Rollback() noexcept final
    {
        m_pHashFile->Rollback();
        m_leaves.clear();
        m_leafMap.clear();
    }

private:
    AppendOnlyFile::Ptr m_pHashFile;
    std::vector<Leaf> m_leaves;
    std::map<mw::Hash, size_t> m_leafMap;
    std::shared_ptr<libmw::IDBWrapper> m_pDatabase;
};

END_NAMESPACE