#pragma once

#include <mw/common/Macros.h>
#include <mw/file/File.h>
#include <mw/models/crypto/Hash.h>
#include <mw/mmr/LeafIndex.h>
#include <mw/traits/Batchable.h>

MMR_NAMESPACE

class LeafSet final : public Traits::IBatchable
{
public:
    using Ptr = std::shared_ptr<LeafSet>;

    static LeafSet::Ptr Load();

    void Add(const LeafIndex& idx);
    void Remove(const LeafIndex& idx);
    bool Contains(const LeafIndex& idx) const noexcept;
    mw::Hash Root(const uint64_t numLeaves) const;
    uint64_t GetSize() const;

    void Rewind(const uint64_t numLeaves, const std::vector<LeafIndex>& leavesToAdd);
    void Commit() final;
    void Rollback() noexcept final;
    void Snapshot(const File& snapshotFile) const;
};

END_NAMESPACE