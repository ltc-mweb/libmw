#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/common/Macros.h>
#include <mw/mmr/Backend.h>
#include <mw/mmr/Node.h>
#include <mw/file/FilePath.h>
#include <mw/file/AppendOnlyFile.h>
#include <mw/db/VectorDB.h>
#include <libmw/interfaces.h>

MMR_NAMESPACE

// TODO: Add pruning support
class FileBackend : public IBackend
{
public:
    static std::shared_ptr<FileBackend> Open(
        const std::string& name,
        const FilePath& chainDir,
        const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper)
    {
        auto path = chainDir.GetChild(name).CreateDirIfMissing();
        return std::make_shared<FileBackend>(
            name,
            AppendOnlyFile::Load(path.GetChild("pmmr_hash.bin")),
            pDBWrapper
        );
    }

    FileBackend(
        const std::string& name,
        const AppendOnlyFile::Ptr& pHashFile,
        const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper)
        : m_name(name), m_pHashFile(pHashFile), m_pDatabase(pDBWrapper) {}

    void AddLeaf(const Leaf& leaf) final
    {
        m_leaves.push_back(leaf.vec());
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

    void Rewind(const LeafIndex& nextLeafIndex, const std::unique_ptr<libmw::IDBBatch>& pBatch) final
    {
        m_pHashFile->Rewind(nextLeafIndex.GetPosition() * 32);
        VectorDB vectorDB(m_name, m_pDatabase.get(), pBatch.get());
        vectorDB.Rewind(nextLeafIndex.GetLeafIndex());
    }

    uint64_t GetNumLeaves() const noexcept final
    {
        VectorDB vectorDB(m_name, m_pDatabase.get());
        return vectorDB.Size() + m_leaves.size();
    }

    mw::Hash GetHash(const Index& idx) const final
    {
        return mw::Hash(m_pHashFile->Read(idx.GetPosition() * mw::Hash::size(), mw::Hash::size()));
    }

    Leaf GetLeaf(const LeafIndex& idx) const final
    {
        const uint64_t leafIndex = idx.GetLeafIndex();
        VectorDB vectorDB(m_name, m_pDatabase.get());
        const uint64_t size = vectorDB.Size();
        std::vector<uint8_t> data;
        if (leafIndex < size) {
            auto pData = vectorDB.Get(leafIndex);
            if (pData) {
                data = std::move(*pData);
            }
        } else {
            data = m_leaves[leafIndex - size];
        }
        return Leaf::Create(idx, std::move(data));
    }

    void Commit(const std::unique_ptr<libmw::IDBBatch>& pBatch = nullptr) final
    {
        m_pHashFile->Commit();
        VectorDB vectorDB(m_name, m_pDatabase.get(), pBatch.get());
        vectorDB.Add(std::move(m_leaves));
    }

    void Rollback() noexcept final
    {
        m_pHashFile->Rollback();
        m_leaves.clear();
    }

private:
    std::string m_name;
    AppendOnlyFile::Ptr m_pHashFile;
    std::vector<std::vector<uint8_t>> m_leaves;
    std::shared_ptr<libmw::IDBWrapper> m_pDatabase;
};

END_NAMESPACE