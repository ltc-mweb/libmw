#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/models/tx/Features.h>
#include <mw/crypto/Hasher.h>
#include <mw/traits/Committed.h>
#include <mw/traits/Hashable.h>
#include <mw/traits/Serializable.h>

////////////////////////////////////////
// OUTPUT IDENTIFIER
////////////////////////////////////////
class OutputId final :
    public Traits::ICommitted,
    public Traits::IHashable,
    public Traits::ISerializable
{
public:
    //
    // Constructors
    //
    OutputId(const EOutputFeatures features, const Commitment& commitment)
        : m_features(features), m_commitment(commitment)
    {
        m_hash = Hashed(*this);
    }
    OutputId(const OutputId& output) = default;
    OutputId(OutputId&& output) noexcept = default;
    OutputId() = default;

    //
    // Operators
    //
    OutputId& operator=(const OutputId& OutputId) = default;
    OutputId& operator=(OutputId&& OutputId) noexcept = default;
    bool operator<(const OutputId& OutputId) const noexcept { return m_hash < OutputId.m_hash; }
    bool operator==(const OutputId& OutputId) const noexcept { return m_hash == OutputId.m_hash; }

    //
    // Getters
    //
    EOutputFeatures GetFeatures() const noexcept { return m_features; }
    const Commitment& GetCommitment() const noexcept final { return m_commitment; }

    bool IsPeggedIn() const noexcept { return (m_features & EOutputFeatures::PEGGED_IN) == EOutputFeatures::PEGGED_IN; }

    //
    // Serialization/Deserialization
    //
    Serializer& Serialize(Serializer& serializer) const noexcept final
    {
        return serializer
            .Append<uint8_t>((uint8_t)m_features)
            .Append(m_commitment);
    }

    static OutputId Deserialize(Deserializer& deserializer)
    {
        const EOutputFeatures features = (EOutputFeatures)deserializer.Read<uint8_t>();
        Commitment commitment = Commitment::Deserialize(deserializer);
        return OutputId(features, std::move(commitment));
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

    mutable mw::Hash m_hash;
};