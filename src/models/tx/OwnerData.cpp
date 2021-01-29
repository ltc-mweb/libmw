#include <mw/models/tx/OwnerData.h>
#include <mw/models/wallet/StealthAddress.h>
#include <mw/crypto/Keys.h>
#include <mw/crypto/Hasher.h>
#include <mw/crypto/Crypto.h>
#include <mw/crypto/Schnorr.h>

OwnerData OwnerData::Create(
    BlindingFactor& blind_out,
    const EOutputFeatures features,
    const SecretKey& sender_privkey,
    const StealthAddress& receiver_addr,
    const uint64_t value)
{
    // Generate 128-bit secret nonce 'n' = Hash128(T_nonce, sender_privkey)
    secret_key_t<16> n = Hashed(EHashTag::NONCE, sender_privkey).data();

    // Calculate unique sending key 's' = H(T_send, A, B, v, n)
    SecretKey s = Hasher(EHashTag::SEND_KEY)
        .Append(receiver_addr.A())
        .Append(receiver_addr.B())
        .Append(value)
        .Append(n)
        .hash();

    // Derive shared secret 't' = H(T_derive, s*A)
    SecretKey t = Hasher(EHashTag::DERIVE)
        .Append(receiver_addr.A().Mul(s))
        .hash();

    // Construct one-time public key for receiver 'Ko' = H(T_outkey, t)*G + B
    PublicKey Ko = PublicKey::From(Hashed(EHashTag::OUT_KEY, t))
        .Add(receiver_addr.B());

    // Key exchange public key 'Ke' = s*B
    PublicKey Ke = receiver_addr.B().Mul(s);

    // Feed the shared secret 't' into a stream cipher (in our case, just a hash function)
    // to derive a blinding factor r and two encryption masks mv (masked value) and mn (masked nonce)
    Deserializer hash64(Hash512(t).vec());
    BlindingFactor r = Crypto::BlindSwitch(hash64.Read<SecretKey>(), value);
    uint64_t mv = hash64.Read<uint64_t>() ^ value;
    BigInt<16> mn = n.GetBigInt() ^ hash64.ReadVector(16);

    // Commitment 'C' = r*G + v*H
    Commitment output_commit = Crypto::CommitBlinded(value, r);

    // Sign the malleable output data
    mw::Hash sig_message = Hasher()
        .Append<uint8_t>(features)
        .Append(Ko)
        .Append(Ke)
        .Append(t[0])
        .Append(mv)
        .Append(mn)
        .hash();
    PublicKey sender_pubkey = Keys::From(sender_privkey).PubKey();
    Signature signature = Schnorr::Sign(sender_privkey.data(), sig_message);

    blind_out = r;
    return OwnerData(
        features,
        std::move(Ko),
        std::move(Ke),
        t[0],
        mv,
        std::move(mn),
        std::move(sender_pubkey),
        std::move(signature)
    );
}

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