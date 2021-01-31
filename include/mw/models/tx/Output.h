#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/models/tx/Features.h>
#include <mw/models/tx/OutputId.h>
#include <mw/models/tx/OwnerData.h>
#include <mw/models/crypto/RangeProof.h>
#include <mw/models/crypto/SecretKey.h>
#include <mw/crypto/Crypto.h>
#include <mw/traits/Committed.h>
#include <mw/traits/Hashable.h>
#include <mw/traits/Serializable.h>

// Forward Declarations
class StealthAddress;

////////////////////////////////////////
// OUTPUT
////////////////////////////////////////
class Output :
    public Traits::ICommitted,
    public Traits::IHashable,
    public Traits::ISerializable
{
public:
    //
    // Constructors
    //
    Output(Commitment&& commitment, OwnerData&& owner_data, const RangeProof::CPtr& pProof)
        : m_commitment(std::move(commitment)), m_ownerData(std::move(owner_data)), m_pProof(pProof)
    {
        m_hash = Hashed(*this);
    }
    Output(const Output& Output) = default;
    Output(Output&& Output) noexcept = default;
    Output() = default;

    //
    // Factory
    //
    static Output Create(
        const EOutputFeatures features,
        const BlindingFactor& blinding_factor,
        const SecretKey& sender_privkey,
        const StealthAddress& receiver_addr,
        const uint64_t amount
    );

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
    EOutputFeatures GetFeatures() const noexcept { return m_ownerData.GetFeatures(); }
    const Commitment& GetCommitment() const noexcept final { return m_commitment; }
    const OwnerData& GetOwnerData() const noexcept { return m_ownerData; }
    const RangeProof::CPtr& GetRangeProof() const noexcept { return m_pProof; }
    const PublicKey& GetSenderPubKey() const noexcept { return m_ownerData.GetSenderPubKey(); }
    const PublicKey& GetReceiverPubKey() const noexcept { return m_ownerData.GetReceiverPubKey(); }
    const PublicKey& GetPubNonce() const noexcept { return m_ownerData.GetPubNonce(); }
    const std::vector<uint8_t>& GetEncrypted() const noexcept { return m_ownerData.GetEncrypted(); }
    const Signature& GetSignature() const noexcept { return m_ownerData.GetSignature(); }

    bool IsPeggedIn() const noexcept { return (GetFeatures() & EOutputFeatures::PEGGED_IN) == EOutputFeatures::PEGGED_IN; }

    OutputId ToIdentifier() const noexcept { return OutputId(GetFeatures(), m_commitment); }

    //
    // Serialization/Deserialization
    //
    Serializer& Serialize(Serializer& serializer) const noexcept final;
    static Output Deserialize(Deserializer& deserializer);

    //
    // Traits
    //
    mw::Hash GetHash() const noexcept final { return m_hash; }

private:
    // The homomorphic commitment representing the output amount
    Commitment m_commitment;

    // Ownership data committed to by the rangeproof
    OwnerData m_ownerData;

    // A proof that the commitment is in the right range
    RangeProof::CPtr m_pProof;

    mw::Hash m_hash;
};