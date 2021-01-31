#include <mw/models/tx/OwnerData.h>
#include <mw/models/wallet/StealthAddress.h>
#include <mw/crypto/Keys.h>
#include <mw/crypto/Hasher.h>
#include <mw/crypto/Crypto.h>

SignedMessage OwnerData::BuildSignedMsg() const noexcept
{
    mw::Hash hashed_msg = Hasher()
        .Append<uint8_t>(m_features)
        .Append(m_receiverPubKey)
        .Append(m_keyExchangePubKey)
        .Append(m_viewTag)
        .Append(m_maskedValue)
        .Append(m_maskedNonce)
        .hash();
    return SignedMessage{ hashed_msg, m_senderPubKey, m_signature };
}

Serializer& OwnerData::Serialize(Serializer& serializer) const noexcept
{
    return serializer
        .Append<uint8_t>(m_features)
        .Append(m_receiverPubKey)
        .Append(m_keyExchangePubKey)
        .Append(m_viewTag)
        .Append(m_maskedValue)
        .Append(m_maskedNonce)
        .Append(m_senderPubKey)
        .Append(m_signature);
}

OwnerData OwnerData::Deserialize(Deserializer& deserializer)
{
    EOutputFeatures features = (EOutputFeatures)deserializer.Read<uint8_t>();
    PublicKey receiverPubKey = PublicKey::Deserialize(deserializer);
    PublicKey keyExchangePubKey = PublicKey::Deserialize(deserializer);
    uint8_t viewTag = deserializer.Read<uint8_t>();
    uint64_t maskedValue = deserializer.Read<uint64_t>();
    BigInt<16> maskedNonce = BigInt<16>::Deserialize(deserializer);
    PublicKey senderPubKey = PublicKey::Deserialize(deserializer);
    Signature signature = Signature::Deserialize(deserializer);

    return OwnerData(
        features,
        std::move(receiverPubKey),
        std::move(keyExchangePubKey),
        viewTag,
        maskedValue,
        std::move(maskedNonce),
        std::move(senderPubKey),
        std::move(signature)
    );
}