#pragma once

#include <mw/consensus/ChainParams.h>
#include <mw/exceptions/DeserializationException.h>
#include <mw/traits/Serializable.h>
#include <boost/container_hash/hash.hpp>
#include <bech32.h>
#include <cstdint>
#include <vector>

class Bech32Address : public Traits::ISerializable
{
public:
    Bech32Address() = default;
    Bech32Address(const std::string& hrp, const std::vector<uint8_t>& address)
        : m_hrp(hrp), m_address(address) { }
    Bech32Address(const std::string& hrp, std::vector<uint8_t>&& address)
        : m_hrp(hrp), m_address(std::move(address)) { }

    static Bech32Address FromString(const std::string& address)
    {
        auto decoded = bech32::Decode(address);
        if (decoded.first.empty() || decoded.second.empty()) {
            ThrowDeserialization("Failed to decode Bech32Address.");
        }

        std::vector<uint8_t> addr = { decoded.second[0] };
        ConvertBits<5, 8, false>([&](uint8_t c) { addr.push_back(c); }, decoded.second.begin() + 1, decoded.second.end());

        return Bech32Address(decoded.first, addr);
    }

    std::string ToString() const
    {
        std::vector<uint8_t> data = { m_address[0] };
        ConvertBits<8, 5, true>([&](uint8_t c) { data.push_back(c); }, m_address.begin() + 1, m_address.end());
        return bech32::Encode(m_hrp, data);
    }

    //
    // Operators
    //
    bool operator!=(const Bech32Address& rhs) const { return m_address != rhs.m_address; }
    bool operator==(const Bech32Address& rhs) const { return m_address == rhs.m_address; }

    const std::vector<uint8_t>& GetAddress() const noexcept { return m_address; }

    //
    // Serialization/Deserialization
    //
    Serializer& Serialize(Serializer& serializer) const noexcept final
    {
        return serializer
            .Append((uint8_t)m_address.size())
            .Append(m_address);
    }

    static Bech32Address Deserialize(Deserializer& deserializer)
    {
        std::string hrp = mw::ChainParams::GetHRP();
        const uint8_t numBytes = deserializer.Read<uint8_t>();
        std::vector<uint8_t> address = deserializer.ReadVector(numBytes);

        return Bech32Address(hrp, std::move(address));
    }

private:
    std::string m_hrp;
    std::vector<uint8_t> m_address;
};

namespace std
{
    template<>
    struct hash<Bech32Address>
    {
        size_t operator()(const Bech32Address& address) const
        {
            return boost::hash_value(address.GetAddress());
        }
    };
}