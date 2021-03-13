#pragma once

#include <mw/file/FilePath.h>
#include <mw/mmr/Index.h>
#include <mw/mmr/LeafIndex.h>
#include <boost/dynamic_bitset.hpp>
#include <memory>

MMR_NAMESPACE

class PruneList
{
public:
    using Ptr = std::shared_ptr<PruneList>;
    using CPtr = std::shared_ptr<const PruneList>;

    static PruneList::Ptr Open(const FilePath& parent_dir, const uint32_t file_index);

    uint64_t GetShift(const mmr::Index& index) const noexcept;
    uint64_t GetShift(const mmr::LeafIndex& index) const noexcept;
    uint64_t GetTotalShift() const noexcept { return m_totalShift; }

    void Commit(const uint32_t file_index, const boost::dynamic_bitset<>& compacted);

private:
    PruneList(const FilePath& dir, boost::dynamic_bitset<>&& compacted, uint64_t total_shift)
        : m_dir(dir), m_compacted(std::move(compacted)), m_totalShift(total_shift) { }

    FilePath m_dir;
    boost::dynamic_bitset<> m_compacted;
    uint64_t m_totalShift;
};

END_NAMESPACE