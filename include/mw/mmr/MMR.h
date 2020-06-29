#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/common/Macros.h>
#include <mw/models/crypto/Hash.h>
#include <mw/traits/Batchable.h>
#include <mw/mmr/Backend.h>
#include <mw/mmr/LeafIndex.h>
#include <mw/mmr/Leaf.h>
#include <mw/mmr/Node.h>

MMR_NAMESPACE

class MMR : public Traits::IBatchable
{
public:
    using Ptr = std::shared_ptr<MMR>;
    using CPtr = std::shared_ptr<const MMR>;

    MMR(const IBackend::Ptr& pBackend) : m_pBackend(pBackend) { }

    void Add(std::vector<uint8_t>&& data);
    void Add(const std::vector<uint8_t>& data) { return Add(std::vector<uint8_t>(data)); }
    Leaf Get(const LeafIndex& leafIdx) const { return m_pBackend->GetLeaf(leafIdx); }

    uint64_t GetNumNodes() const noexcept;
    void Rewind(const uint64_t numNodes);

    //
    // Unlike a Merkle tree, a MMR generally has no single root so we need a method to compute one.
    // The process we use is called "bagging the peaks." We first identify the peaks (nodes with no parents).
    // We then "bag" them by hashing them iteratively from the right, using the total size of the MMR as prefix. 
    //
    mw::Hash Root() const;

    void Commit() final { m_pBackend->Commit(); }
    void Rollback() noexcept final { m_pBackend->Rollback(); }

private:
    IBackend::Ptr m_pBackend;
};

END_NAMESPACE