#include <mw/crypto/Crypto.h>
#include <mw/exceptions/CryptoException.h>
#include <mw/common/Logger.h>

#include <Crypto/Blake2.h>
#include <Crypto/sha256.h>
#include <Crypto/ripemd160.h>
#include <Crypto/hmac_sha256.h>
#include <Crypto/hmac_sha512.h>
#include <Crypto/aes.h>
#include <Crypto/siphash.h>
#include <Crypto/crypto_scrypt.h>
#include <cassert>

// Secp256k1
#include "Context.h"
#include "Bulletproofs.h"
#include "MuSig.h"
#include "Pedersen.h"
#include "PublicKeys.h"
#include "Schnorr.h"
#include "ConversionUtil.h"

#ifdef _WIN32
#pragma comment(lib, "crypt32")
#endif

Locked<Context> SECP256K1_CONTEXT(std::make_shared<Context>());

BigInt<32> Crypto::Blake2b(const std::vector<uint8_t>& input)
{
    BigInt<32> result;
    const int status = blake2b(result.data(), 32, input.data(), input.size(), nullptr, 0);
    if (status != 0)
    {
        ThrowCrypto_F("blake2b failed with status: {}", status);
    }

    return result;
}

BigInt<32> Crypto::Blake2b(
    const std::vector<uint8_t>& key,
    const std::vector<uint8_t>& input)
{
    BigInt<32> result;
    const int status = blake2b(result.data(), 32, input.data(), input.size(), key.data(), key.size());
    if (status != 0)
    {
        ThrowCrypto_F("blake2b failed with status: {}", status);
    }

    return result;
}

BigInt<32> Crypto::SHA256(const std::vector<uint8_t>& input)
{
    BigInt<32> result;
    CSHA256()
        .Write(input.data(), input.size())
        .Finalize(result.data());
    return result;
}

BigInt<64> Crypto::SHA512(const std::vector<uint8_t>& input)
{
    BigInt<64> result;
    CSHA512()
        .Write(input.data(), input.size())
        .Finalize(result.data());
    return result;
}

BigInt<20> Crypto::RipeMD160(const std::vector<uint8_t>& input)
{
    BigInt<20> result;
    CRIPEMD160()
        .Write(input.data(), input.size())
        .Finalize(result.data());
    return result;
}

BigInt<32> Crypto::HMAC_SHA256(
    const std::vector<uint8_t>& key,
    const std::vector<uint8_t>& data)
{
    BigInt<32> result;
    CHMAC_SHA256(key.data(), key.size())
        .Write(data.data(), data.size())
        .Finalize(result.data());
    return result;
}

BigInt<64> Crypto::HMAC_SHA512(
    const std::vector<uint8_t>& key,
    const std::vector<uint8_t>& data)
{
    BigInt<64> result;
    CHMAC_SHA512(key.data(), key.size())
        .Write(data.data(), data.size())
        .Finalize(result.data());
    return result;
}

Commitment Crypto::CommitTransparent(const uint64_t value)
{
    return Pedersen(SECP256K1_CONTEXT).PedersenCommit(value, BigInt<32>::ValueOf(0));
}

Commitment Crypto::CommitBlinded(
    const uint64_t value,
    const BlindingFactor& blindingFactor)
{
    return Pedersen(SECP256K1_CONTEXT).PedersenCommit(value, blindingFactor);
}

Commitment Crypto::AddCommitments(
    const std::vector<Commitment>& positive,
    const std::vector<Commitment>& negative)
{
    const Commitment zeroCommitment(BigInt<33>::ValueOf(0));

    std::vector<Commitment> sanitizedPositive;
    std::copy_if(
        positive.cbegin(),
        positive.cend(),
        std::back_inserter(sanitizedPositive),
        [&zeroCommitment](const auto& positiveCommitment) {
            return positiveCommitment != zeroCommitment;
        }
    );

    std::vector<Commitment> sanitizedNegative;
    std::copy_if(
        negative.cbegin(),
        negative.cend(),
        std::back_inserter(sanitizedNegative),
        [&zeroCommitment](const auto& negativeCommitment) {
            return negativeCommitment != zeroCommitment;
        }
    );

    return Pedersen(SECP256K1_CONTEXT).PedersenCommitSum(
        sanitizedPositive,
        sanitizedNegative
    );
}

BlindingFactor Crypto::AddBlindingFactors(
    const std::vector<BlindingFactor>& positive,
    const std::vector<BlindingFactor>& negative)
{
    BlindingFactor zeroBlindingFactor(ZERO_HASH);

    std::vector<BlindingFactor> sanitizedPositive;
    std::copy_if(
        positive.cbegin(),
        positive.cend(),
        std::back_inserter(sanitizedPositive),
        [&zeroBlindingFactor](const auto& positiveBlind) {
            return positiveBlind != zeroBlindingFactor;
        }
    );

    std::vector<BlindingFactor> sanitizedNegative;
    std::copy_if(
        negative.cbegin(),
        negative.cend(),
        std::back_inserter(sanitizedNegative),
        [&zeroBlindingFactor](const auto& negativeBlind) {
            return negativeBlind != zeroBlindingFactor;
        }
    );

    if (sanitizedPositive.empty() && sanitizedNegative.empty())
    {
        return zeroBlindingFactor;
    }

    return Pedersen(SECP256K1_CONTEXT).PedersenBlindSum(sanitizedPositive, sanitizedNegative);
}

SecretKey Crypto::BlindSwitch(const SecretKey& secretKey, const uint64_t amount)
{
    return Pedersen(SECP256K1_CONTEXT).BlindSwitch(secretKey, amount);
}

SecretKey Crypto::AddPrivateKeys(const SecretKey& secretKey1, const SecretKey& secretKey2)
{
    SecretKey result(secretKey1.vec());

    const int tweakResult = secp256k1_ec_privkey_tweak_add(
        SECP256K1_CONTEXT.Read()->Get(),
        (uint8_t*)result.data(),
        secretKey2.data()
    );
    if (tweakResult == 1)
    {
        return result;
    }

    ThrowCrypto("secp256k1_ec_privkey_tweak_add failed");
}

RangeProof::CPtr Crypto::GenerateRangeProof(
    const uint64_t amount,
    const SecretKey& key,
    const SecretKey& privateNonce,
    const SecretKey& rewindNonce,
    const ProofMessage& proofMessage)
{
    return Bulletproofs(SECP256K1_CONTEXT).GenerateRangeProof(
        amount,
        key,
        privateNonce,
        rewindNonce,
        proofMessage
    );
}

std::unique_ptr<RewoundProof> Crypto::RewindRangeProof(
    const Commitment& commitment,
    const RangeProof& rangeProof,
    const SecretKey& nonce)
{
    return Bulletproofs(SECP256K1_CONTEXT).RewindProof(commitment, rangeProof, nonce);
}

bool Crypto::VerifyRangeProofs(
    const std::vector<std::pair<Commitment, RangeProof::CPtr>>& rangeProofs)
{
    return Bulletproofs(SECP256K1_CONTEXT).VerifyBulletproofs(rangeProofs);
}

uint64_t Crypto::SipHash24(
    const uint64_t k0,
    const uint64_t k1,
    const std::vector<uint8_t>& data)
{
    const std::vector<uint64_t> key = { k0, k1 };

    return siphash24(key.data(), data.data(), data.size());
}

std::vector<uint8_t> Crypto::AES256_Encrypt(
    const SecureVector& input,
    const SecretKey& key,
    const BigInt<16>& iv)
{
    assert(input.size() <= INT_MAX);

    std::vector<uint8_t> ciphertext;

    // max ciphertext len for a n bytes of plaintext is n + AES_BLOCKSIZE bytes
    ciphertext.resize(input.size() + AES_BLOCKSIZE);

    AES256CBCEncrypt enc(key.data(), iv.data(), true);
    const size_t nLen = enc.Encrypt(input.data(), (int)input.size(), ciphertext.data());
    if (nLen < input.size())
    {
        ThrowCrypto("Failed to encrypt");
    }

    ciphertext.resize(nLen);

    return ciphertext;
}

SecureVector Crypto::AES256_Decrypt(
    const std::vector<uint8_t>& ciphertext,
    const SecretKey& key,
    const BigInt<16>& iv)
{
    assert(ciphertext.size() <= INT_MAX);

    // plaintext will always be equal to or lesser than length of ciphertext
    SecureVector plaintext(ciphertext.size());

    AES256CBCDecrypt dec(key.data(), iv.data(), true);
    size_t nLen = dec.Decrypt(ciphertext.data(), (int)ciphertext.size(), plaintext.data());
    if (nLen == 0)
    {
        ThrowCrypto("Failed to decrypt");
    }

    plaintext.resize(nLen);

    return plaintext;
}

SecretKey Crypto::PBKDF(
    const char* password,
    const size_t passwordLen,
    const std::vector<uint8_t>& salt,
    const ScryptParameters& parameters)
{
    SecureVector buffer(64);
    const int result = crypto_scrypt(
        (const uint8_t*)password,
        passwordLen,
        salt.data(),
        salt.size(),
        parameters.N,
        parameters.r,
        parameters.p,
        buffer.data(),
        buffer.size()
    );
    if (result == 0)
    {
        SecretKey result;
        blake2b(result.data(), 32, buffer.data(), buffer.size(), nullptr, 0);
        return result;
    }

    ThrowCrypto("Scrypt failed");
}

PublicKey Crypto::CalculatePublicKey(const SecretKey& privateKey)
{
    return PublicKeys(SECP256K1_CONTEXT).CalculatePublicKey(privateKey);
}

PublicKey Crypto::AddPublicKeys(const std::vector<PublicKey>& publicKeys)
{
    return PublicKeys(SECP256K1_CONTEXT).PublicKeySum(publicKeys);
}

PublicKey Crypto::ToPublicKey(const Commitment& commitment)
{
    return ConversionUtil(SECP256K1_CONTEXT).ToPublicKey(commitment);
}

Signature Crypto::BuildSignature(
    const SecretKey& secretKey,
    const Hash& messageHash)
{
    return Schnorr(SECP256K1_CONTEXT).Sign(
        secretKey,
        messageHash
    );    
}

CompactSignature Crypto::CalculatePartialSignature(
    const SecretKey& secretKey,
    const SecretKey& secretNonce,
    const PublicKey& sumPubKeys,
    const PublicKey& sumPubNonces,
    const Hash& message)
{
    return MuSig(SECP256K1_CONTEXT).CalculatePartialSignature(
        secretKey,
        secretNonce,
        sumPubKeys,
        sumPubNonces,
        message
    );
}

Signature Crypto::AggregateSignatures(
    const std::vector<CompactSignature>& signatures,
    const PublicKey& sumPubNonces)
{
    return MuSig(SECP256K1_CONTEXT).AggregateSignatures(signatures, sumPubNonces);
}

bool Crypto::VerifyPartialSignature(
    const CompactSignature& partialSignature,
    const PublicKey& publicKey,
    const PublicKey& sumPubKeys,
    const PublicKey& sumPubNonces,
    const Hash& message)
{
    return MuSig(SECP256K1_CONTEXT).VerifyPartialSignature(
        partialSignature,
        publicKey,
        sumPubKeys,
        sumPubNonces,
        message
    );
}

bool Crypto::VerifyAggregateSignature(
    const Signature& aggregateSignature,
    const PublicKey sumPubKeys,
    const Hash& message)
{
    return Schnorr(SECP256K1_CONTEXT).Verify(
        aggregateSignature,
        sumPubKeys,
        message
    );
}

bool Crypto::VerifyKernelSignatures(
    const std::vector<const Signature*>& signatures,
    const std::vector<const Commitment*>& publicKeys,
    const std::vector<const Hash*>& messages)
{
    return Schnorr(SECP256K1_CONTEXT).BatchVerify(
        signatures,
        publicKeys,
        messages
    );
}

SecretKey Crypto::GenerateSecureNonce()
{
    return MuSig(SECP256K1_CONTEXT).GenerateSecureNonce();
}