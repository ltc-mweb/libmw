#pragma once

#include <mw/models/crypto/PublicKey.h>
#include <mw/models/crypto/Signature.h>
#include <mw/models/crypto/SignedMessage.h>
#include <mw/models/tx/Features.h>
#include <mw/traits/Serializable.h>
#include <mw/crypto/Crypto.h>
#include <mw/crypto/Schnorr.h>

class OwnerData : public Traits::ISerializable
{
public:
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

    SignedMessage GetSignedMsg() const noexcept
    {
        auto serialized_msg = Serializer()
            .Append<uint8_t>(m_features)
            .Append(m_receiverPubKey)
            .Append(m_pubNonce)
            .Append<uint8_t>((uint8_t)m_encrypted.size())
            .Append(m_encrypted)
            .vec();
        return SignedMessage{ Hashed(serialized_msg), m_senderPubKey, m_signature };
    }

    bool TryDecrypt(const SecretKey& secretKey, std::vector<uint8_t>& decrypted) const noexcept
    {
        try {
            decrypted = Crypto::AES256_Decrypt(m_encrypted, secretKey, BigInt<16>()); // TODO: Use IV?
            return true;
        } catch (...) { }

        return false;
    }

    //
    // Serialization/Deserialization
    //
    Serializer& Serialize(Serializer& serializer) const noexcept final
    {
        return serializer
            .Append<uint8_t>(m_features)
            .Append(m_senderPubKey)
            .Append(m_receiverPubKey)
            .Append(m_pubNonce)
            .Append<uint8_t>((uint8_t)m_encrypted.size())
            .Append(m_encrypted)
            .Append(m_signature);
    }

    static OwnerData Deserialize(Deserializer& deserializer)
    {
        EOutputFeatures features = (EOutputFeatures)deserializer.Read<uint8_t>();
        PublicKey senderPubKey = PublicKey::Deserialize(deserializer);
        PublicKey receiverPubKey = PublicKey::Deserialize(deserializer);
        PublicKey pubNonce = PublicKey::Deserialize(deserializer);
        const uint8_t size = deserializer.Read<uint8_t>();
        std::vector<uint8_t> encrypted = deserializer.ReadVector(size);
        Signature signature = Signature::Deserialize(deserializer);

        return OwnerData(
            features,
            std::move(senderPubKey),
            std::move(receiverPubKey),
            std::move(pubNonce),
            std::move(encrypted),
            std::move(signature)
        );
    }

private:
    // Options for an output's structure or use
    EOutputFeatures m_features;
    PublicKey m_senderPubKey;
    PublicKey m_receiverPubKey;
    PublicKey m_pubNonce;
    std::vector<uint8_t> m_encrypted;
    Signature m_signature;
};