#pragma once

#include <mw/models/crypto/BlindingFactor.h>
#include <mw/models/crypto/PublicKey.h>
#include <mw/models/crypto/SecretKey.h>
#include <mw/models/crypto/Signature.h>
#include <mw/models/crypto/SignedMessage.h>
#include <mw/models/tx/Features.h>
#include <mw/traits/Serializable.h>

// Forward Declarations
class StealthAddress;

class OwnerData : public Traits::ISerializable
{
public:
    //
    // Constructors
    //
    OwnerData() = default;
    OwnerData(const OwnerData&) = default;
    OwnerData(OwnerData&&) = default;
    OwnerData(
        EOutputFeatures features,
        PublicKey&& senderPubKey,
        PublicKey&& receiverPubKey,
        PublicKey&& pubNonce,
        std::vector<uint8_t>&& encrypted,
        Signature&& signature
    )
        : m_features(features),
        m_senderPubKey(std::move(senderPubKey)),
        m_receiverPubKey(std::move(receiverPubKey)),
        m_pubNonce(std::move(pubNonce)),
        m_encrypted(std::move(encrypted)),
        m_signature(std::move(signature)) { }

    //
    // Factory
    //
    static OwnerData Create(
        const EOutputFeatures features,
        const SecretKey& sender_privkey,
        const StealthAddress& receiver_addr,
        const BlindingFactor& blinding_factor,
        const uint64_t amount
    );

    //
    // Operators
    //
    OwnerData& operator=(const OwnerData& rhs) = default;
    OwnerData& operator=(OwnerData&& rhs) noexcept = default;
    bool operator==(const OwnerData& rhs) const noexcept
    {
        if (&rhs == this) {
            return true;
        }

        return m_senderPubKey == rhs.m_senderPubKey
            && m_receiverPubKey == rhs.m_receiverPubKey
            && m_pubNonce == rhs.m_pubNonce
            && m_encrypted == rhs.m_encrypted
            && m_signature == rhs.m_signature;
    }

    //
    // Getters
    //
    EOutputFeatures GetFeatures() const noexcept { return m_features; }
    const PublicKey& GetSenderPubKey() const noexcept { return m_senderPubKey; }
    const PublicKey& GetReceiverPubKey() const noexcept { return m_receiverPubKey; }
    const PublicKey& GetPubNonce() const noexcept { return m_pubNonce; }
    const std::vector<uint8_t>& GetEncrypted() const noexcept { return m_encrypted; }
    const Signature& GetSignature() const noexcept { return m_signature; }

    SignedMessage GetSignedMsg() const noexcept;
    bool TryDecrypt(const SecretKey& secretKey, std::vector<uint8_t>& decrypted) const noexcept;

    //
    // Serialization/Deserialization
    //
    Serializer& Serialize(Serializer& serializer) const noexcept final;
    static OwnerData Deserialize(Deserializer& deserializer);

private:
    // Options for an output's structure or use
    EOutputFeatures m_features;
    PublicKey m_senderPubKey;
    PublicKey m_receiverPubKey;
    PublicKey m_pubNonce;
    std::vector<uint8_t> m_encrypted;
    Signature m_signature;
};