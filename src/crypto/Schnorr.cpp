#include <mw/crypto/Schnorr.h>
#include "Context.h"
#include "ConversionUtil.h"

#include <mw/common/Logger.h>
#include <mw/exceptions/CryptoException.h>
#include <mw/util/VectorUtil.h>

static Locked<Context> SCHNORR_CONTEXT(std::make_shared<Context>());

static constexpr uint64_t MAX_WIDTH = 1 << 20;
static constexpr size_t SCRATCH_SPACE_SIZE = 256 * MAX_WIDTH;

Signature Schnorr::Sign(
    const uint8_t* secretKey,
    const mw::Hash& message)
{
    secp256k1_schnorrsig signature;
    const int signedResult = secp256k1_schnorrsig_sign(
        SCHNORR_CONTEXT.Write()->Randomized(),
        &signature,
        nullptr,
        message.data(),
        secretKey,
        nullptr,
        nullptr
    );
    if (signedResult != 1) {
        ThrowCrypto("Failed to sign message.");
    }

    return ConversionUtil(SCHNORR_CONTEXT).ToSignature(signature);
}

bool Schnorr::Verify(
    const Signature& signature,
    const PublicKey& sumPubKeys,
    const mw::Hash& message)
{
    secp256k1_pubkey parsedPubKey = ConversionUtil(SCHNORR_CONTEXT).ToSecp256k1(sumPubKeys);

    const int verifyResult = secp256k1_aggsig_verify_single(
        SCHNORR_CONTEXT.Read()->Get(),
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

bool Schnorr::BatchVerify(const std::vector<SignedMessage>& signatures)
{
    // Parse pubkeys
    std::vector<secp256k1_pubkey> parsedPubKeys;
    std::transform(
        signatures.cbegin(), signatures.cend(),
        std::back_inserter(parsedPubKeys),
        [](const SignedMessage& signed_message) -> secp256k1_pubkey {
            return ConversionUtil(SCHNORR_CONTEXT).ToSecp256k1(signed_message.GetPublicKey());
        }
    );
    std::vector<secp256k1_pubkey*> pubKeyPtrs = VectorUtil::ToPointerVec(parsedPubKeys);

    // Parse signatures
    std::vector<secp256k1_schnorrsig> parsedSignatures;
    std::transform(
        signatures.cbegin(), signatures.cend(),
        std::back_inserter(parsedSignatures),
        [](const SignedMessage& signed_message) -> secp256k1_schnorrsig {
            return ConversionUtil(SCHNORR_CONTEXT).ToSecp256k1(signed_message.GetSignature());
        }
    );
    std::vector<secp256k1_schnorrsig*> signaturePtrs = VectorUtil::ToPointerVec(parsedSignatures);

    // Transform messages
    std::vector<const uint8_t*> messageData;
    std::transform(
        signatures.cbegin(), signatures.cend(),
        std::back_inserter(messageData),
        [](const SignedMessage& signed_message) { return signed_message.GetMsgHash().data(); }
    );

    secp256k1_scratch_space* pScratchSpace = secp256k1_scratch_space_create(
        SCHNORR_CONTEXT.Read()->Get(),
        SCRATCH_SPACE_SIZE
    );
    const int verifyResult = secp256k1_schnorrsig_verify_batch(
        SCHNORR_CONTEXT.Read()->Get(),
        pScratchSpace,
        signaturePtrs.data(),
        messageData.data(),
        pubKeyPtrs.data(),
        signatures.size()
    );
    secp256k1_scratch_space_destroy(pScratchSpace);

    return verifyResult == 1;
}