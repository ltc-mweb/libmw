#include <mw/mmr/PruneList.h>
#include <mw/file/File.h>
#include <mw/util/BitUtil.h>

mmr::PruneList::Ptr mmr::PruneList::Open(const FilePath& parent_dir, const uint32_t file_index)
{
    File file(parent_dir.GetChild(StringUtil::Format("prun{:0>6}.dat", file_index)));

    boost::dynamic_bitset<> bitset;
    if (file.Exists()) {
        std::vector<uint8_t> bytes = file.ReadBytes();
        bitset.reserve(bytes.size() * 8);

        for (uint8_t byte : bytes) {
            for (uint8_t i = 0; i < 8; i++) {
                bitset.push_back(byte & (0x80 >> i));
            }
        }
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

void mmr::PruneList::Commit(const uint32_t file_index, const boost::dynamic_bitset<>& compacted)
{
    File file(m_dir.GetChild(StringUtil::Format("prun{:0>6}.dat", file_index)));

    std::vector<uint8_t> bytes((compacted.size() + 7) / 8);

    for (size_t i = 0; i < bytes.size(); i++) {
        for (uint8_t j = 0; j < 8; j++) {
            size_t bit_index = (i * 8) + j;
            if (compacted.size() > bit_index && compacted.test(bit_index)) {
                bytes[i] |= (0x80 >> j);
            }
        }
    }

    file.Write(bytes);

    m_compacted = compacted;
    m_totalShift = compacted.count();
}