#pragma once

#include <boost/dynamic_bitset.hpp>
#include <vector>

struct BitSet
{
    boost::dynamic_bitset<> bitset;

    static BitSet From(const std::vector<uint8_t>& bytes)
    {
        BitSet ret;
        ret.bitset.reserve(bytes.size() * 8);

        for (uint8_t byte : bytes) {
            for (uint8_t i = 0; i < 8; i++) {
                ret.bitset.push_back(byte & (0x80 >> i));
            }
        }

        return ret;
    }

    std::vector<uint8_t> bytes() const noexcept
    {
        std::vector<uint8_t> bytes((bitset.size() + 7) / 8);

        for (size_t i = 0; i < bytes.size(); i++) {
            for (uint8_t j = 0; j < 8; j++) {
                size_t bit_index = (i * 8) + j;
                if (bitset.size() > bit_index && bitset.test(bit_index)) {
                    bytes[i] |= (0x80 >> j);
                }
            }
        }

        return bytes;
    }

    bool test(uint64_t idx) const noexcept { return bitset.size() > idx && bitset.test(idx); }
    uint64_t count() const noexcept { return bitset.count(); }
    uint64_t size() const noexcept { return bitset.size(); }

    void set(uint64_t idx, bool val = true) noexcept { bitset.set(idx, val); }
};