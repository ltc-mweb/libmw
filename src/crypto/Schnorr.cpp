#include "Schnorr.h"
#include "ConversionUtil.h"

#include <mw/common/Logger.h>
#include <mw/crypto/Random.h>
#include <mw/exceptions/CryptoException.h>
#include <mw/util/VectorUtil.h>

const uint64_t MAX_WIDTH = 1 << 20;
const size_t SCRATCH_SPACE_SIZE = 256 * MAX_WIDTH;

Signature Schnorr::Sign(
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

bool Schnorr::Verify(
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

bool Schnorr::BatchVerify(
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
        [this](const Commitment* commitment) -> secp256k1_pubkey {
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