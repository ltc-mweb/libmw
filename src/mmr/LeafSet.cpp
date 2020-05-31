#include <mw/mmr/LeafSet.h>

// TODO: Implement

MMR_NAMESPACE

LeafSet::Ptr LeafSet::Load()
{
    return std::make_shared<LeafSet>();
}

void LeafSet::Add(const LeafIndex& idx)
{

}

void LeafSet::Remove(const LeafIndex& idx)
{

}

bool LeafSet::Contains(const LeafIndex& idx) const noexcept
{
    return false;
}

Hash LeafSet::Root(const uint64_t numLeaves) const
{
    return {};
}

uint64_t LeafSet::GetSize() const
{
    return 0;
}

void LeafSet::Rewind(const uint64_t numLeaves, const std::vector<LeafIndex>& leavesToAdd)
{

}

void LeafSet::Commit()
{

}

void LeafSet::Rollback() noexcept
{

}

void LeafSet::Snapshot(const File& snapshotFile) const
{

}

END_NAMESPACE