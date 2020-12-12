#include <mw/crypto/Schnorr.h>
#include "Context.h"
#include "ConversionUtil.h"

#include <mw/common/Logger.h>
#include <mw/exceptions/CryptoException.h>
#include <mw/util/VectorUtil.h>

Locked<Context> SCHNORR_CONTEXT(std::make_shared<Context>());

const uint64_t MAX_WIDTH = 1 << 20;
const size_t SCRATCH_SPACE_SIZE = 256 * MAX_WIDTH;

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

bool Schnorr::BatchVerify(const std::vector<std::tuple<Signature, Commitment, mw::Hash>>& signatures)
{
    std::vector<const Signature*> signature_ptrs;
    std::vector<const Commitment*> commitment_ptrs;
    std::vector<const mw::Hash*> message_ptrs;

    for (const auto& signature : signatures)
    {
        signature_ptrs.push_back(&std::get<0>(signature));
        commitment_ptrs.push_back(&std::get<1>(signature));
        message_ptrs.push_back(&std::get<2>(signature));
    }

    return BatchVerify(signature_ptrs, commitment_ptrs, message_ptrs);
}

bool Schnorr::BatchVerify(
    const std::vector<const Signature*>& signatures,
    const std::vector<const Commitment*>& commitments,
    const std::vector<const mw::Hash*>& messages)
{
    assert(signatures.size() == commitments.size());
    assert(commitments.size() == messages.size());

    std::vector<secp256k1_pubkey> parsedPubKeys;
    std::transform(
        commitments.cbegin(), commitments.cend(),
        std::back_inserter(parsedPubKeys),
        [](const Commitment* commitment) -> secp256k1_pubkey {
            PublicKey publicKey = ConversionUtil(SCHNORR_CONTEXT).ToPublicKey(*commitment);
            return ConversionUtil(SCHNORR_CONTEXT).ToSecp256k1(publicKey);
        }
    );

    std::vector<secp256k1_pubkey*> pubKeyPtrs = VectorUtil::ToPointerVec(parsedPubKeys);

    std::vector<secp256k1_schnorrsig> parsedSignatures = ConversionUtil(SCHNORR_CONTEXT).ToSecp256k1(signatures);
    std::vector<secp256k1_schnorrsig*> signaturePtrs = VectorUtil::ToPointerVec(parsedSignatures);

    std::vector<const unsigned char*> messageData;
    std::transform(
        messages.cbegin(), messages.cend(),
        std::back_inserter(messageData),
        [](const mw::Hash* pMessage) { return pMessage->data(); }
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