#include "MuSig.h"
#include "ConversionUtil.h"

#include <mw/common/Logger.h>
#include <mw/crypto/Random.h>
#include <mw/exceptions/CryptoException.h>
#include <mw/util/VectorUtil.h>

const uint64_t MAX_WIDTH = 1 << 20;
const size_t SCRATCH_SPACE_SIZE = 256 * MAX_WIDTH;

SecretKey MuSig::GenerateSecureNonce() const
{
    SecretKey nonce;
    const SecretKey seed = Random::CSPRNG<32>();
    const int result = secp256k1_aggsig_export_secnonce_single(
        m_context.Read()->Get(),
        nonce.data(),
        seed.data()
    );
    if (result != 1)
    {
        ThrowCrypto_F("secp256k1_aggsig_export_secnonce_single failed with error: {}", result);
    }

    return nonce;
}

CompactSignature MuSig::CalculatePartialSignature(
    const SecretKey& secretKey,
    const SecretKey& secretNonce,
    const PublicKey& sumPubKeys,
    const PublicKey& sumPubNonces,
    const mw::Hash& message)
{
    secp256k1_pubkey pubKeyForE = ConversionUtil(m_context).ToSecp256k1(sumPubKeys);
    secp256k1_pubkey pubNoncesForE = ConversionUtil(m_context).ToSecp256k1(sumPubNonces);

    const SecretKey randomSeed = Random::CSPRNG<32>();

    secp256k1_ecdsa_signature signature;
    const int signedResult = secp256k1_aggsig_sign_single(
        m_context.Write()->Randomized(),
        signature.data,
        message.data(),
        secretKey.data(),
        secretNonce.data(),
        nullptr,
        &pubNoncesForE,
        &pubNoncesForE,
        &pubKeyForE,
        randomSeed.data()
    );
    if (signedResult != 1)
    {
        ThrowCrypto("Failed to calculate partial signature.");
    }

    return ConversionUtil(m_context).ToCompact(signature);
}

bool MuSig::VerifyPartialSignature(
    const CompactSignature& partialSignature,
    const PublicKey& publicKey,
    const PublicKey& sumPubKeys,
    const PublicKey& sumPubNonces,
    const mw::Hash& message) const
{
    secp256k1_ecdsa_signature signature = ConversionUtil(m_context).ToSecp256k1(partialSignature);

    secp256k1_pubkey pubkey = ConversionUtil(m_context).ToSecp256k1(publicKey);
    secp256k1_pubkey sumPubKey = ConversionUtil(m_context).ToSecp256k1(sumPubKeys);
    secp256k1_pubkey sumNoncesPubKey = ConversionUtil(m_context).ToSecp256k1(sumPubNonces);

    const int verifyResult = secp256k1_aggsig_verify_single(
        m_context.Read()->Get(),
        signature.data,
        message.data(),
        &sumNoncesPubKey,
        &pubkey,
        &sumPubKey,
        nullptr,
        true
    );

    return verifyResult == 1;
}

Signature MuSig::AggregateSignatures(
    const std::vector<CompactSignature>& signatures,
    const PublicKey& sumPubNonces) const
{
    assert(!signatures.empty());

    secp256k1_pubkey pubNonces = ConversionUtil(m_context).ToSecp256k1(sumPubNonces);

    std::vector<secp256k1_ecdsa_signature> parsedSignatures = ConversionUtil(m_context).ToSecp256k1(signatures);
    std::vector<secp256k1_ecdsa_signature*> signaturePtrs = VectorUtil::ToPointerVec(parsedSignatures);

    secp256k1_ecdsa_signature aggregatedSignature;
    const int result = secp256k1_aggsig_add_signatures_single(
        m_context.Read()->Get(),
        aggregatedSignature.data,
        (const unsigned char**)signaturePtrs.data(),
        signaturePtrs.size(),
        &pubNonces
    );
    if (result != 1)
    {
        ThrowCrypto("Failed to aggregate signatures");
    }

    return Signature(aggregatedSignature.data);
}