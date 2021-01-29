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

// TODO: Inherit SignedMessage?
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
        PublicKey&& receiverPubKey,
        PublicKey&& keyExchangePubKey,
        uint8_t viewTag,
        uint64_t maskedValue,
        BigInt<16>&& maskedNonce,
        PublicKey&& senderPubKey,
        Signature&& signature
    )
        : m_features(features),
        m_receiverPubKey(std::move(receiverPubKey)),
        m_keyExchangePubKey(std::move(keyExchangePubKey)),
        m_viewTag(viewTag),
        m_maskedValue(maskedValue),
        m_maskedNonce(std::move(maskedNonce)),
        m_senderPubKey(std::move(senderPubKey)),
        m_signature(std::move(signature)) { }

    //
    // Factory
    //
    static OwnerData Create(
        BlindingFactor& blind_out,
        const EOutputFeatures features,
        const SecretKey& sender_privkey,
        const StealthAddress& receiver_addr,
        const uint64_t value
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
            && m_keyExchangePubKey == rhs.m_keyExchangePubKey
            && m_viewTag == rhs.m_viewTag
            && m_maskedValue == rhs.m_maskedValue
            && m_maskedNonce == rhs.m_maskedNonce
            && m_senderPubKey == rhs.m_senderPubKey
            && m_signature == rhs.m_signature;
    }

    //
    // Getters
    //
    EOutputFeatures GetFeatures() const noexcept { return m_features; }
    const PublicKey& GetReceiverPubKey() const noexcept { return m_receiverPubKey; }
    const PublicKey& GetKeyExchangePubKey() const noexcept { return m_keyExchangePubKey; }
    uint8_t GetViewTag() const noexcept { return m_viewTag; }
    uint64_t GetMaskedValue() const noexcept { return m_maskedValue; }
    const BigInt<16>& GetMaskedNonce() const noexcept { return m_maskedNonce; }
    const PublicKey& GetSenderPubKey() const noexcept { return m_senderPubKey; }
    const Signature& GetSignature() const noexcept { return m_signature; }

    SignedMessage BuildSignedMsg() const noexcept;

    //
    // Serialization/Deserialization
    //
    Serializer& Serialize(Serializer& serializer) const noexcept final;
    static OwnerData Deserialize(Deserializer& deserializer);

private:
    EOutputFeatures m_features;
    PublicKey m_receiverPubKey;
    PublicKey m_keyExchangePubKey;
    uint8_t m_viewTag;
    uint64_t m_maskedValue;
    BigInt<16> m_maskedNonce;
    PublicKey m_senderPubKey;
    Signature m_signature;
};