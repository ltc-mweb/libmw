#pragma once

#include <mw/config/ChainParams.h>
#include <mw/exceptions/DeserializationException.h>
#include <mw/traits/Serializable.h>
#include <mw/traits/Jsonable.h>
#include <boost/container_hash/hash.hpp>
#include <bech32.h>
#include <cstdint>
#include <vector>

class Bech32Address : public Traits::ISerializable, public Traits::IJsonable
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
        if (decoded.first.empty()) {
            ThrowDeserialization("Failed to decode Bech32Address.");
        }

        std::vector<uint8_t> addr;
        ConvertBits<5, 8, false>([&](uint8_t c) { addr.push_back(c); }, decoded.second.begin(), decoded.second.end());

        return Bech32Address(decoded.first, addr);
    }

    std::string ToString() const
    {
        std::vector<uint8_t> data;
        ConvertBits<8, 5, true>([&](uint8_t c) { data.push_back(c); }, m_address.begin(), m_address.end());
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

    json ToJSON() const noexcept final
    {
        std::string address = ToString();
        return json(address);
    }

    static Bech32Address FromJSON(const Json& json)
    {
        std::string addressStr = json.Get<std::string>();
        return FromString(addressStr);
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