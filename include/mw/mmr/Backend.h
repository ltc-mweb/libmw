#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/common/Macros.h>
#include <mw/models/crypto/Hash.h>
#include <mw/traits/Batchable.h>
#include <mw/mmr/Index.h>
#include <mw/mmr/LeafIndex.h>
#include <mw/mmr/Leaf.h>
#include <memory>

MMR_NAMESPACE

class IBackend : public Traits::IBatchable
{
public:
    using Ptr = std::shared_ptr<IBackend>;
    using CPtr = std::shared_ptr<const IBackend>;

    virtual ~IBackend() = default;

    virtual void AddLeaf(const Leaf& leaf) = 0;
    virtual void AddHash(const mw::Hash& hash) = 0;
    virtual void Rewind(const LeafIndex& nextLeafIndex) = 0;

    virtual uint64_t GetNumLeaves() const noexcept = 0;
    virtual mw::Hash GetHash(const Index& idx) const = 0;
    virtual Leaf GetLeaf(const LeafIndex& idx) const = 0;

    virtual LeafIndex GetNextLeaf() const noexcept { return LeafIndex::At(GetNumLeaves()); }
};

END_NAMESPACE