#include <mw/mmr/LeafSet.h>
#include <mw/crypto/Hasher.h>

MMR_NAMESPACE

LeafSet::Ptr LeafSet::Open(const FilePath& leafset_dir)
{
	File file = leafset_dir.GetChild("leafset.bin");
    if (!file.Exists()) {
        file.Create();
    }

    mmr::LeafIndex nextLeafIdx = mmr::LeafIndex::At(0);
    if (file.GetSize() < 8) {
        file.Write(Serializer().Append<uint64_t>(0).vec());
    } else {
        Deserializer deserializer{ file.ReadBytes(0, 8) };
        nextLeafIdx = mmr::LeafIndex::At(deserializer.Read<uint64_t>());
    }

    MemMap mappedFile{ file };
    mappedFile.Map();
	return std::shared_ptr<LeafSet>(new LeafSet{ std::move(mappedFile), nextLeafIdx });
}

void LeafSet::ApplyUpdates(const mmr::LeafIndex& nextLeafIdx, const std::unordered_map<uint64_t, uint8_t>& modifiedBytes)
{
	for (auto byte : modifiedBytes) {
		m_modifiedBytes[byte.first + 8] = byte.second;
	}

    // In case of rewind, make sure to clear everything above the new next
    for (size_t idx = nextLeafIdx.Get(); idx < m_nextLeafIdx.Get(); idx++) {
        Remove(mmr::LeafIndex::At(idx));
    }

    m_nextLeafIdx = nextLeafIdx;

    Flush();
}

void LeafSet::Flush()
{
    m_mmap.Unmap();

    std::vector<uint8_t> nextLeafIdxBytes = Serializer().Append<uint64_t>(m_nextLeafIdx.Get()).vec();
    assert(nextLeafIdxBytes.size() == 8);

    for (uint8_t i = 0; i < 8; i++) {
        m_modifiedBytes[i] = nextLeafIdxBytes[i];
    }

    m_mmap.GetFile().WriteBytes(m_modifiedBytes);
    m_mmap.Map();

    m_modifiedBytes.clear();
}

uint8_t LeafSet::GetByte(const uint64_t byteIdx) const
{
    // Offset by 8 bytes, since first 8 bytes in file represent the next leaf index
    const uint64_t byteIdxWithOffset = byteIdx + 8;

    auto iter = m_modifiedBytes.find(byteIdxWithOffset);
    if (iter != m_modifiedBytes.cend())
    {
        return iter->second;
    }
    else if (byteIdxWithOffset < m_mmap.size())
    {
        return m_mmap.ReadByte(byteIdxWithOffset);
    }

    return 0;
}

void LeafSet::SetByte(const uint64_t byteIdx, const uint8_t value)
{
	m_modifiedBytes[byteIdx + 8] = value;
}

END_NAMESPACE