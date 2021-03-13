#include <mw/mmr/PruneList.h>
#include <mw/file/File.h>

mmr::PruneList::Ptr mmr::PruneList::Open(const FilePath& parent_dir, const uint32_t file_index)
{
    File file(parent_dir.GetChild(StringUtil::Format("prun{:0>6}.dat", file_index)));

    BitSet bitset;
    if (file.Exists()) {
        bitset = BitSet::From(file.ReadBytes());
    }

    uint64_t total_shift = bitset.count();
    return std::shared_ptr<PruneList>(new PruneList(parent_dir, std::move(bitset), total_shift));
}

// TODO: This is an inefficient algorithm.
// A shift cache with an efficient "nearest-neighbor" searching algorithm would be better.
uint64_t mmr::PruneList::GetShift(const mmr::Index& index) const noexcept
{
    uint64_t shift = 0;
    for (uint64_t idx = 0; idx < index.GetPosition(); idx++) {
        if (idx >= m_compacted.size()) {
            break;
        }

        if (m_compacted.test(idx)) {
            ++shift;
        }
    }

    return shift;
}

uint64_t mmr::PruneList::GetShift(const mmr::LeafIndex& index) const noexcept
{
    return GetShift(index.GetNodeIndex());
}

void mmr::PruneList::Commit(const uint32_t file_index, const BitSet& compacted)
{
    File file(m_dir.GetChild(StringUtil::Format("prun{:0>6}.dat", file_index)));
    file.Write(compacted.bytes());

    m_compacted = compacted;
    m_totalShift = compacted.count();
}