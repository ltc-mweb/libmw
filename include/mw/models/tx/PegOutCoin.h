#pragma once

#include <mw/models/crypto/Bech32Address.h>
#include <mw/traits/Serializable.h>
#include <mw/traits/Printable.h>
#include <mw/traits/Jsonable.h>

//
// Represents coins being pegged in, i.e. moved from canonical chain to the extension block.
//
class PegOutCoin : public Traits::ISerializable, public Traits::IJsonable, public Traits::IPrintable
{
public:
    PegOutCoin(const uint64_t amount, const Bech32Address& address)
        : m_amount(amount), m_address(address) { }
    PegOutCoin(const uint64_t amount, Bech32Address&& address)
        : m_amount(amount), m_address(std::move(address)) { }

    bool operator==(const PegOutCoin& rhs) const noexcept
    {
        return m_amount == rhs.m_amount && m_address == rhs.m_address;
    }

    uint64_t GetAmount() const noexcept { return m_amount; }
    const Bech32Address& GetAddress() const noexcept { return m_address; }

    //
    // Serialization/Deserialization
    //
    Serializer& Serialize(Serializer& serializer) const noexcept final
    {
        return serializer
            .Append(m_amount)
            .Append(m_address);
    }

    static PegOutCoin Deserialize(Deserializer& deserializer)
    {
        uint64_t amount = deserializer.Read<uint64_t>();
        Bech32Address address = Bech32Address::Deserialize(deserializer);

        return PegOutCoin(amount, std::move(address));
    }

    json ToJSON() const noexcept final
    {
        return json({
            { "amount", m_amount },
            { "address", m_address }
        });
    }

    static PegOutCoin FromJSON(const Json& json)
    {
        uint64_t amount = json.GetOr<uint64_t>("amount", 0);
        Bech32Address commitment = json.GetRequired<Bech32Address>("address");

        return PegOutCoin(amount, std::move(commitment));
    }

    std::string Format() const noexcept final
    {
        return std::string("PegOutCoin(address: ") + m_address.ToString() + ", amount: " + std::to_string(m_amount) + ")";
    }

private:
    uint64_t m_amount;
    Bech32Address m_address;
};