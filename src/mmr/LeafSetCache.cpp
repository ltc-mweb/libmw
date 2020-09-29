#include <mw/mmr/LeafSet.h>
#include <mw/crypto/Hasher.h>

MMR_NAMESPACE

//uint64_t LeafSetCache::GetSize() const
//{
//	size_t size = m_pBacked->GetSize();
//	for (auto iter = m_modifiedBytes.cbegin(); iter != m_modifiedBytes.cend(); iter++)
//	{
//		if (iter->first >= size) {
//			size = iter->first + 1;
//		}
//	}
//
//	return size * 8;
//}

void LeafSetCache::ApplyUpdates(const mmr::LeafIndex& nextLeafIdx, const std::unordered_map<uint64_t, uint8_t>& modifiedBytes)
{
	m_nextLeafIdx = nextLeafIdx;

	for (auto byte : modifiedBytes) {
		m_modifiedBytes[byte.first] = byte.second;
	}
}

void LeafSetCache::Flush()
{
	m_pBacked->ApplyUpdates(m_nextLeafIdx, m_modifiedBytes);
	m_modifiedBytes.clear();
}

uint8_t LeafSetCache::GetByte(const uint64_t byteIdx) const
{
	auto iter = m_modifiedBytes.find(byteIdx);
	if (iter != m_modifiedBytes.cend())
	{
		return iter->second;
	}

	return m_pBacked->GetByte(byteIdx);
}

void LeafSetCache::SetByte(const uint64_t byteIdx, const uint8_t value)
{
	m_modifiedBytes[byteIdx] = value;
}

END_NAMESPACE