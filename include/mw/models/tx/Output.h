#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/models/tx/Features.h>
#include <mw/models/tx/OutputId.h>
#include <mw/models/crypto/RangeProof.h>
#include <mw/crypto/Crypto.h>
#include <mw/traits/Committed.h>
#include <mw/traits/Hashable.h>
#include <mw/traits/Serializable.h>
#include <mw/traits/Jsonable.h>

////////////////////////////////////////
// OUTPUT
////////////////////////////////////////
class Output :
    public Traits::ICommitted,
    public Traits::IHashable,
    public Traits::ISerializable,
    public Traits::IJsonable
{
public:
    //
    // Constructors
    //
    Output(const EOutputFeatures features, Commitment&& commitment, const RangeProof::CPtr& pProof)
        : m_features(features), m_commitment(std::move(commitment)), m_pProof(pProof)
    {
        m_hash = Hashed(*this);
    }
    Output(const Output& Output) = default;
    Output(Output&& Output) noexcept = default;
    Output() = default;

    //
    // Destructor
    //
    virtual ~Output() = default;

    //
    // Operators
    //
    Output& operator=(const Output& Output) = default;
    Output& operator=(Output&& Output) noexcept = default;
    bool operator<(const Output& Output) const noexcept { return m_hash < Output.m_hash; }
    bool operator==(const Output& Output) const noexcept { return m_hash == Output.m_hash; }

    //
    // Getters
    //
    EOutputFeatures GetFeatures() const noexcept { return m_features; }
    const Commitment& GetCommitment() const noexcept final { return m_commitment; }
    const RangeProof::CPtr& GetRangeProof() const noexcept { return m_pProof; }

    bool IsCoinbase() const noexcept { return (m_features & EOutputFeatures::COINBASE_OUTPUT) == EOutputFeatures::COINBASE_OUTPUT; }

    OutputId ToIdentifier() const noexcept { return OutputId(m_features, m_commitment); }

    //
    // Serialization/Deserialization
    //
    Serializer& Serialize(Serializer& serializer) const noexcept final
    {
        return serializer
            .Append<uint8_t>((uint8_t)m_features)
            .Append(m_commitment)
            .Append(m_pProof);
    }

    static Output Deserialize(Deserializer& deserializer)
    {
        const EOutputFeatures features = (EOutputFeatures)deserializer.Read<uint8_t>();
        Commitment commitment = Commitment::Deserialize(deserializer);
        RangeProof::CPtr pProof = std::make_shared<const RangeProof>(RangeProof::Deserialize(deserializer));
        return Output(features, std::move(commitment), pProof);
    }

    json ToJSON() const noexcept final
    {
        return json({
            {"features", OutputFeatures::ToString(m_features)},
            {"commit", m_commitment},
            {"proof", m_pProof}
        });
    }

    static Output FromJSON(const Json& json)
    {
        return Output(
            OutputFeatures::FromString(json.GetRequired<std::string>("features")),
            json.GetRequired<Commitment>("commit"),
            std::make_shared<const RangeProof>(json.GetRequired<RangeProof>("proof"))
        );
    }

    //
    // Traits
    //
    mw::Hash GetHash() const noexcept final { return m_hash; }

private:
    // Options for an output's structure or use
    EOutputFeatures m_features;

    // The homomorphic commitment representing the output amount
    Commitment m_commitment;

    // A proof that the commitment is in the right range
    RangeProof::CPtr m_pProof;

    mutable mw::Hash m_hash;
};