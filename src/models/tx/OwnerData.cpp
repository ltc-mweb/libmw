#include <mw/models/tx/OwnerData.h>
#include <mw/models/wallet/StealthAddress.h>
#include <mw/crypto/Keys.h>
#include <mw/crypto/Hasher.h>
#include <mw/crypto/Crypto.h>
#include <mw/crypto/Schnorr.h>

OwnerData OwnerData::Create(
    const EOutputFeatures features,
    const SecretKey& sender_privkey,
    const StealthAddress& receiver_addr,
    const BlindingFactor& blinding_factor,
    const uint64_t amount)
{
    PublicKey sender_pubkey = Keys::From(sender_privkey).PubKey();
    SecretKey r = Random::CSPRNG<32>();
    PublicKey R = Keys::From(r).PubKey();
    PublicKey rA = Keys::From(receiver_addr.A()).Mul(r).PubKey();
    PublicKey receiver_pubkey = Keys::From(Hashed(rA)).Add(receiver_addr.B()).PubKey();

    // TODO: Include version in plaintext
    std::vector<uint8_t> plaintext = Serializer()
        .Append(blinding_factor)
        .Append<uint64_t>(amount)
        .vec();
    SecretKey shared_secret = Hashed(Keys::From(receiver_addr.B()).Mul(sender_privkey).PubKey());
    std::vector<uint8_t> encrypted_data = Crypto::AES256_Encrypt(plaintext, shared_secret, BigInt<16>());

    mw::Hash sig_message = Hasher()
        .Append<uint8_t>(features)
        .Append(receiver_pubkey)
        .Append(R)
        .Append<uint8_t>((uint8_t)encrypted_data.size())
        .Append(encrypted_data)
        .hash();
    Signature signature = Schnorr::Sign(sender_privkey.data(), sig_message);

    return OwnerData(
        features,
        std::move(sender_pubkey),
        std::move(receiver_pubkey),
        std::move(R),
        std::move(encrypted_data),
        std::move(signature)
    );
}

SignedMessage OwnerData::BuildSignedMsg() const noexcept
{
    mw::Hash hashed_msg = Hasher()
        .Append<uint8_t>(m_features)
        .Append(m_receiverPubKey)
        .Append(m_pubNonce)
        .Append<uint8_t>((uint8_t)m_encrypted.size())
        .Append(m_encrypted)
        .hash();
    return SignedMessage{ hashed_msg, m_senderPubKey, m_signature };
}

bool OwnerData::TryDecrypt(const SecretKey& secretKey, std::vector<uint8_t>& decrypted) const noexcept
{
    try {
        decrypted = Crypto::AES256_Decrypt(m_encrypted, secretKey, BigInt<16>()); // TODO: Use IV?
        return true;
    }
    catch (...) {}

    return false;
}

Serializer& OwnerData::Serialize(Serializer& serializer) const noexcept
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

OwnerData OwnerData::Deserialize(Deserializer& deserializer)
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