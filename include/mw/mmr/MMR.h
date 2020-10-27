#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/common/Macros.h>
#include <mw/common/Logger.h>
#include <mw/models/crypto/Hash.h>
#include <mw/traits/Batchable.h>
#include <mw/traits/Serializable.h>
#include <mw/mmr/Backend.h>
#include <mw/mmr/LeafIndex.h>
#include <mw/mmr/Leaf.h>
#include <mw/mmr/Node.h>
#include <libmw/interfaces/db_interface.h>

MMR_NAMESPACE

class IMMR
{
public:
    using Ptr = std::shared_ptr<IMMR>;
    using CPtr = std::shared_ptr<const IMMR>;
    using UPtr = std::unique_ptr<IMMR>;

    virtual LeafIndex AddLeaf(std::vector<uint8_t>&& data) = 0;
    LeafIndex Add(const std::vector<uint8_t>& data) { return AddLeaf(std::vector<uint8_t>(data)); }
    LeafIndex Add(const Traits::ISerializable& serializable) { return AddLeaf(serializable.Serialized()); }

    virtual Leaf GetLeaf(const LeafIndex& leafIdx) const = 0;
    virtual mw::Hash GetHash(const Index& idx) const = 0;
    virtual LeafIndex GetNextLeafIdx() const noexcept = 0;
    uint64_t GetNumLeaves() const noexcept { return GetNextLeafIdx().GetLeafIndex(); }

    virtual void Rewind(const uint64_t numLeaves) = 0;
    void Rewind(const LeafIndex& nextLeaf) { Rewind(nextLeaf.GetLeafIndex()); }

    //
    // Unlike a Merkle tree, a MMR generally has no single root so we need a method to compute one.
    // The process we use is called "bagging the peaks." We first identify the peaks (nodes with no parents).
    // We then "bag" them by hashing them iteratively from the right, using the total size of the MMR as prefix. 
    //
    mw::Hash Root() const
    {
        const uint64_t size = GetNextLeafIdx().GetPosition();
        if (size == 0)
        {
            return ZERO_HASH;
        }

        // Find the "peaks"
        std::vector<uint64_t> peakIndices;

        uint64_t peakSize = BitUtil::FillOnesToRight(size);
        uint64_t numLeft = size;
        uint64_t sumPrevPeaks = 0;
        while (peakSize != 0)
        {
            if (numLeft >= peakSize)
            {
                peakIndices.push_back(sumPrevPeaks + peakSize - 1);
                sumPrevPeaks += peakSize;
                numLeft -= peakSize;
            }

            peakSize >>= 1;
        }

        assert(numLeft == 0);

        // Bag 'em
        mw::Hash hash = ZERO_HASH;
        for (auto iter = peakIndices.crbegin(); iter != peakIndices.crend(); iter++)
        {
            mw::Hash peakHash = GetHash(Index::At(*iter));
            if (hash == ZERO_HASH)
            {
                hash = peakHash;
            }
            else
            {
                hash = Node::CreateParent(Index::At(size), peakHash, hash).GetHash();
            }
        }

        return hash;
    }

    virtual void BatchWrite(
        const LeafIndex& firstLeafIdx,
        const std::vector<Leaf>& leaves,
        const std::unique_ptr<libmw::IDBBatch>& pBatch) = 0;
};

class MMR : public Traits::IBatchable, public IMMR
{
public:
    using Ptr = std::shared_ptr<MMR>;
    using CPtr = std::shared_ptr<const MMR>;

    MMR(const IBackend::Ptr& pBackend) : m_pBackend(pBackend) { }

    LeafIndex AddLeaf(std::vector<uint8_t>&& data) final;

    Leaf GetLeaf(const LeafIndex& leafIdx) const final { return m_pBackend->GetLeaf(leafIdx); }
    mw::Hash GetHash(const Index& idx) const final { return m_pBackend->GetHash(idx); }
    LeafIndex GetNextLeafIdx() const noexcept final { return m_pBackend->GetNextLeaf(); }

    uint64_t GetNumLeaves() const noexcept;
    uint64_t GetNumNodes() const noexcept;
    void Rewind(const uint64_t numLeaves) final;

    //mw::Hash Root() const final;

    void Commit() final { m_pBackend->Commit(); }
    void Rollback() noexcept final { m_pBackend->Rollback(); }

    void BatchWrite(
        const LeafIndex& firstLeafIdx,
        const std::vector<Leaf>& leaves,
        const std::unique_ptr<libmw::IDBBatch>& pBatch) final;

private:
    IBackend::Ptr m_pBackend;
};

class MMRCache : public IMMR
{
public:
    using Ptr = std::shared_ptr<MMRCache>;
    using UPtr = std::unique_ptr<MMRCache>;

    MMRCache(const IMMR::Ptr& pBacked)
        : m_pBase(pBacked), m_firstLeaf(pBacked->GetNextLeafIdx()){ }

    LeafIndex AddLeaf(std::vector<uint8_t>&& data) final
    {
        LeafIndex leafIdx = LeafIndex::At(m_firstLeaf.GetLeafIndex() + m_leaves.size());
        Leaf leaf = Leaf::Create(leafIdx, std::move(data));

        m_nodes.push_back(leaf.GetHash());

        auto rightHash = leaf.GetHash();
        auto nextIdx = leaf.GetNodeIndex().GetNext();
        while (!nextIdx.IsLeaf())
        {
            const mw::Hash leftHash = GetHash(nextIdx.GetLeftChild());
            const Node node = Node::CreateParent(nextIdx, leftHash, rightHash);

            m_nodes.push_back(node.GetHash());
            rightHash = node.GetHash();
            nextIdx = nextIdx.GetNext();
        }

        m_leaves.push_back(std::move(leaf));
        return leafIdx;
    }

    Leaf GetLeaf(const LeafIndex& leafIdx) const final
    {
        if (leafIdx < m_firstLeaf) {
            return m_pBase->GetLeaf(leafIdx);
        }

        const uint64_t cacheIdx = leafIdx.GetLeafIndex() - m_firstLeaf.GetLeafIndex();
        if (cacheIdx > m_leaves.size()) {
            throw std::out_of_range("Attempting to access non-existent leaf");
        }

        return m_leaves[cacheIdx];
    }

    LeafIndex GetNextLeafIdx() const noexcept final
    {
        if (m_leaves.empty()) {
            return m_firstLeaf;
        } else {
            return m_leaves.back().GetLeafIndex().Next();
        }
    }

    mw::Hash GetHash(const Index& idx) const final
    {
        if (idx < m_firstLeaf.GetPosition()) {
            return m_pBase->GetHash(idx);
        } else {
            const uint64_t vecIdx = idx.GetPosition() - m_firstLeaf.GetPosition();
            assert(m_nodes.size() > vecIdx);
            return m_nodes[vecIdx];
        }
    }

    void Rewind(const uint64_t numLeaves) final
    {
        LOG_TRACE_F("MMRCache: Rewinding to {}", numLeaves);

        LeafIndex nextLeaf = LeafIndex::At(numLeaves);
        if (nextLeaf <= m_firstLeaf) {
            m_firstLeaf = nextLeaf;
            m_leaves.clear();
            m_nodes.clear();
        } else if (!m_leaves.empty()) {
            auto iter = m_leaves.begin();
            while (iter != m_leaves.end() && iter->GetLeafIndex() < nextLeaf) {
                iter++;
            }

            if (iter != m_leaves.end()) {
                m_leaves.erase(iter, m_leaves.end());
            }

            const uint64_t numNodes = GetNextLeafIdx().GetPosition() - m_firstLeaf.GetPosition();
            if (m_nodes.size() > numNodes) {
                m_nodes.erase(m_nodes.begin() + numNodes, m_nodes.end());
            }
        }
    }

    void BatchWrite(
        const LeafIndex& firstLeafIdx,
        const std::vector<Leaf>& leaves,
        const std::unique_ptr<libmw::IDBBatch>&) final
    {
        LOG_TRACE_F("MMRCache: Writing batch {}", firstLeafIdx.GetLeafIndex());
        Rewind(firstLeafIdx.GetLeafIndex());
        for (const Leaf& leaf : leaves)
        {
            Add(leaf.vec());
        }
    }

    void Flush(const std::unique_ptr<libmw::IDBBatch>& pBatch = nullptr)
    {
        LOG_TRACE_F("MMRCache: Flushing {} leaves at {}", m_leaves.size(), m_firstLeaf.GetLeafIndex());
        m_pBase->BatchWrite(m_firstLeaf, m_leaves, pBatch);
        m_firstLeaf = GetNextLeafIdx();
        m_leaves.clear();
        m_nodes.clear();
    }

private:
    IMMR::Ptr m_pBase;
    LeafIndex m_firstLeaf;
    std::vector<Leaf> m_leaves;
    std::vector<mw::Hash> m_nodes;
};

END_NAMESPACE