#pragma once

#include <caches/Cache.h>
#include <mw/models/crypto/RangeProof.h>
#include <mw/models/crypto/Commitment.h>
#include <mutex>

class BulletProofsCache
{
public:
    BulletProofsCache() : m_bulletproofsCache(3000) { }

    void AddToCache(const std::tuple<Commitment, RangeProof::CPtr, std::vector<uint8_t>>& proof_tuple)
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        m_bulletproofsCache.Put(std::get<0>(proof_tuple), proof_tuple);
    }

    void AddToCache(const Commitment& commitment, const RangeProof::CPtr& pRangeProof, const std::vector<uint8_t>& data)
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        m_bulletproofsCache.Put(commitment, std::make_tuple(commitment, pRangeProof, data));
    }

    bool WasAlreadyVerified(const Commitment& commitment, const RangeProof::CPtr& pRangeProof, const std::vector<uint8_t>& data) const
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (m_bulletproofsCache.Cached(commitment))
        {
            auto cached_value = m_bulletproofsCache.Get(commitment);

            return std::get<0>(cached_value) == commitment && *std::get<1>(cached_value) == *pRangeProof && std::get<2>(cached_value) == data;
        }

        return false;
    }

    bool WasAlreadyVerified(const std::tuple<Commitment, RangeProof::CPtr, std::vector<uint8_t>>& proof_tuple) const
    {
        return WasAlreadyVerified(std::get<0>(proof_tuple), std::get<1>(proof_tuple), std::get<2>(proof_tuple));
    }

private:
    mutable std::mutex m_mutex;
    mutable LRUCache<Commitment, std::tuple<Commitment, RangeProof::CPtr, std::vector<uint8_t>>> m_bulletproofsCache;
};