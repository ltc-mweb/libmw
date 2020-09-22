#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/models/crypto/BigInteger.h>
#include <mw/models/crypto/SecretKey.h>
#include <mw/traits/Serializable.h>

class BlindingFactor : public Traits::ISerializable
{
public:
    //
    // Constructors
    //
    BlindingFactor() = default;
    BlindingFactor(BigInt<32>&& value) : m_value(std::move(value)) { }
    BlindingFactor(const BigInt<32>& value) : m_value(value) { }
    BlindingFactor(const SecretKey& key) : m_value(key.GetBigInt()) { }
    BlindingFactor(const BlindingFactor& other) = default;
    BlindingFactor(BlindingFactor&& other) noexcept = default;

    //
    // Destructor
    //
    virtual ~BlindingFactor() = default;

    //
    // Operators
    //
    BlindingFactor& operator=(const BlindingFactor& other) = default;
    BlindingFactor& operator=(BlindingFactor&& other) noexcept = default;
    bool operator<(const BlindingFactor& rhs) const noexcept { return m_value < rhs.GetBigInt(); }
    bool operator!=(const BlindingFactor& rhs) const noexcept { return m_value != rhs.GetBigInt(); }
    bool operator==(const BlindingFactor& rhs) const noexcept { return m_value == rhs.GetBigInt(); }

    //
    // Getters
    //
    const BigInt<32>& GetBigInt() const noexcept { return m_value; }
    const std::vector<uint8_t>& vec() const noexcept { return m_value.vec(); }
    const uint8_t* data() const noexcept { return m_value.data(); }
    uint8_t* data() noexcept { return m_value.data(); }
    size_t size() const noexcept { return m_value.size(); }
    bool IsZero() const noexcept { return m_value.IsZero(); }

    //
    // Serialization/Deserialization
    //
    Serializer& Serialize(Serializer& serializer) const noexcept final
    {
        return m_value.Serialize(serializer);
    }

    static BlindingFactor Deserialize(Deserializer& deserializer)
    {
        return BlindingFactor(BigInt<32>::Deserialize(deserializer));
    }

    std::string ToHex() const { return m_value.ToHex(); }
    static BlindingFactor FromHex(const std::string& hex) { return BlindingFactor(BigInt<32>::FromHex(hex)); }

    //
    // Converts BlindingFactor to SecretKey.
    // WARNING: BlindingFactor is unusable after calling this.
    //
    SecretKey ToSecretKey()
    {
        return SecretKey(std::move(m_value));
    }

private:
    // The 32 byte blinding factor.
    BigInt<32> m_value;
};