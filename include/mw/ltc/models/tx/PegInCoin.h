#pragma once

#include <mw/core/models/crypto/Commitment.h>
#include <mw/core/traits/Serializable.h>
#include <mw/core/traits/Printable.h>
#include <mw/core/traits/Jsonable.h>

//
// Represents coins being pegged in, i.e. moved from canonical chain to the extension block.
//
class PegInCoin : public Traits::ISerializable, public Traits::IJsonable
{
public:
    PegInCoin(const uint64_t amount, const Commitment& commitment)
        : m_amount(amount), m_commitment(commitment) { }
    PegInCoin(const uint64_t amount, Commitment&& commitment)
        : m_amount(amount), m_commitment(std::move(commitment)) { }

    uint64_t GetAmount() const noexcept { return m_amount; }
    const Commitment& GetCommitment() const noexcept { return m_commitment; }

    //
    // Serialization/Deserialization
    //
    Serializer& Serialize(Serializer& serializer) const noexcept final
    {
        return serializer
            .Append(m_amount)
            .Append(m_commitment);
    }

    static PegInCoin Deserialize(Deserializer& deserializer)
    {
        uint64_t amount = deserializer.Read<uint64_t>();
        Commitment commitment = Commitment::Deserialize(deserializer);

        return PegInCoin(amount, std::move(commitment));
    }

    json ToJSON() const noexcept final
    {
        return json({
            { "amount", m_amount },
            { "commitment", m_commitment }
        });
    }

    static PegInCoin FromJSON(const Json& json)
    {
        uint64_t amount = json.GetOr<uint64_t>("amount", 0);
        Commitment commitment = json.GetRequired<Commitment>("commitment");

        return PegInCoin(amount, std::move(commitment));
    }

private:
    uint64_t m_amount;
    Commitment m_commitment;
};