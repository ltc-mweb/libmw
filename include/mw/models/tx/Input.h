#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/models/tx/Features.h>
#include <mw/crypto/Crypto.h>
#include <mw/traits/Committed.h>
#include <mw/traits/Hashable.h>
#include <mw/traits/Serializable.h>
#include <mw/traits/Jsonable.h>

////////////////////////////////////////
// INPUT
////////////////////////////////////////
class Input :
    public Traits::ICommitted,
    public Traits::IHashable,
    public Traits::ISerializable,
    public Traits::IJsonable
{
public:
    //
    // Constructors
    //
    Input(const EOutputFeatures features, Commitment&& commitment)
        : m_features(features), m_commitment(std::move(commitment))
    {
        Serializer serializer;
        Serialize(serializer);
        m_hash = Crypto::Blake2b(serializer.vec());
    }
    Input(const Input& input) = default;
    Input(Input&& input) noexcept = default;
    Input() = default;

    //
    // Destructor
    //
    virtual ~Input() = default;

    //
    // Operators
    //
    Input& operator=(const Input& input) = default;
    Input& operator=(Input&& input) noexcept = default;
    bool operator<(const Input& input) const noexcept { return m_hash < input.m_hash; }
    bool operator==(const Input& input) const noexcept { return m_hash == input.m_hash; }

    //
    // Getters
    //
    EOutputFeatures GetFeatures() const noexcept { return m_features; }
    const Commitment& GetCommitment() const noexcept final { return m_commitment; }

    bool IsCoinbase() const noexcept { return (m_features & EOutputFeatures::COINBASE_OUTPUT) == EOutputFeatures::COINBASE_OUTPUT; }

    //
    // Serialization/Deserialization
    //
    Serializer& Serialize(Serializer& serializer) const noexcept final
    {
        return serializer
            .Append<uint8_t>((uint8_t)m_features)
            .Append(m_commitment);
    }

    static Input Deserialize(Deserializer& deserializer)
    {
        const EOutputFeatures features = (EOutputFeatures)deserializer.Read<uint8_t>();
        Commitment commitment = Commitment::Deserialize(deserializer);
        return Input(features, std::move(commitment));
    }

    json ToJSON() const noexcept final
    {
        return json({
            {"features", OutputFeatures::ToString(m_features)},
            {"commit", m_commitment}
        });
    }

    static Input FromJSON(const Json& json)
    {
        return Input(
            OutputFeatures::FromString(json.GetRequired<std::string>("features")),
            json.GetRequired<Commitment>("commit")
        );
    }

    //
    // Traits
    //
    Hash GetHash() const noexcept final { return m_hash; }

private:
    // The features of the output being spent. 
    // We will check maturity for coinbase output.
    EOutputFeatures m_features;

    // The commit referencing the output being spent.
    Commitment m_commitment;

    mutable Hash m_hash;
};