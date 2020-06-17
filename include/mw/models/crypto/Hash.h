#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/models/crypto/BigInteger.h>
#include <mw/util/BitUtil.h>
#include <boost/container_hash/hash.hpp>

using Hash = BigInt<32>;

class HASH
{
public:
    static inline const Hash ZERO = Hash::ValueOf(0);
    static inline const uint8_t LENGTH = 32;
};

#define ZERO_HASH HASH::ZERO

namespace std
{
    template<>
    struct hash<Hash>
    {
        size_t operator()(const Hash& hash) const
        {
            return boost::hash_value(hash.vec());
        }
    };
}