#include "AggSig.h"
#include "Pedersen.h"
#include "ConversionUtil.h"

#include <mw/common/Logger.h>
#include <mw/crypto/Random.h>
#include <mw/exceptions/CryptoException.h>
#include <mw/util/VectorUtil.h>

const uint64_t MAX_WIDTH = 1 << 20;
const size_t SCRATCH_SPACE_SIZE = 256 * MAX_WIDTH;

SecretKey AggSig::GenerateSecureNonce() const
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

Signature AggSig::SignMessage(
    const SecretKey& secretKey,
    const Hash& message)
{
    const SecretKey randomSeed = Random::CSPRNG<32>();

    secp256k1_schnorrsig signature;
    const int signedResult = secp256k1_schnorrsig_sign(
        m_context.Write()->Randomized(),
        &signature,
        nullptr,
        message.data(),
        secretKey.data(),
        nullptr,
        nullptr
    );
    if (signedResult != 1)
    {
        ThrowCrypto("Failed to sign message.");
    }

    return ConversionUtil(m_context).ToSignature(signature); 
}

CompactSignature AggSig::CalculatePartialSignature(
    const SecretKey& secretKey,
    const SecretKey& secretNonce,
    const PublicKey& sumPubKeys,
    const PublicKey& sumPubNonces,
    const Hash& message)
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

bool AggSig::VerifyPartialSignature(
    const CompactSignature& partialSignature,
    const PublicKey& publicKey,
    const PublicKey& sumPubKeys,
    const PublicKey& sumPubNonces,
    const Hash& message) const
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

Signature AggSig::AggregateSignatures(
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

bool AggSig::VerifyAggregateSignatures(
    const std::vector<const Signature*>& signatures,
    const std::vector<const Commitment*>& commitments,
    const std::vector<const Hash*>& messages) const
{
    assert(signatures.size() == commitments.size());
    assert(commitments.size() == messages.size());

    std::vector<secp256k1_pubkey> parsedPubKeys;
    std::transform(
        commitments.cbegin(), commitments.cend(),
        std::back_inserter(parsedPubKeys),
        [this](const Commitment* commitment) {
            PublicKey publicKey = ConversionUtil(m_context).ToPublicKey(*commitment);
            return ConversionUtil(m_context).ToSecp256k1(publicKey);
        }
    );

    std::vector<secp256k1_pubkey*> pubKeyPtrs = VectorUtil::ToPointerVec(parsedPubKeys);

    std::vector<secp256k1_schnorrsig> parsedSignatures = ConversionUtil(m_context).ToSecp256k1(signatures);
    std::vector<secp256k1_schnorrsig*> signaturePtrs = VectorUtil::ToPointerVec(parsedSignatures);

    std::vector<const unsigned char*> messageData;
    std::transform(
        messages.cbegin(), messages.cend(),
        std::back_inserter(messageData),
        [](const Hash* pMessage) { return pMessage->data(); }
    );

    secp256k1_scratch_space* pScratchSpace = secp256k1_scratch_space_create(
        m_context.Read()->Get(),
        SCRATCH_SPACE_SIZE
    );
    const int verifyResult = secp256k1_schnorrsig_verify_batch(
        m_context.Read()->Get(),
        pScratchSpace,
        signaturePtrs.data(),
        messageData.data(),
        pubKeyPtrs.data(),
        signatures.size()
    );
    secp256k1_scratch_space_destroy(pScratchSpace);

    return verifyResult == 1;
}

bool AggSig::VerifyAggregateSignature(
    const Signature& signature,
    const PublicKey& sumPubKeys,
    const Hash& message) const
{
    secp256k1_pubkey parsedPubKey = ConversionUtil(m_context).ToSecp256k1(sumPubKeys);

    const int verifyResult = secp256k1_aggsig_verify_single(
        m_context.Read()->Get(),
        signature.data(),
        message.data(),
        nullptr,
        &parsedPubKey,
        &parsedPubKey,
        nullptr,
        false
    );

    return verifyResult == 1;
}