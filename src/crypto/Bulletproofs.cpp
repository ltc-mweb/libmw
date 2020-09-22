#include "Bulletproofs.h"
#include "ConversionUtil.h"

#include <mw/crypto/Random.h>
#include <mw/exceptions/CryptoException.h>
#include <mw/util/VectorUtil.h>

const uint64_t MAX_WIDTH = 1 << 20;
const size_t SCRATCH_SPACE_SIZE = 256 * MAX_WIDTH;

bool Bulletproofs::VerifyBulletproofs(const std::vector<std::tuple<Commitment, RangeProof::CPtr, std::vector<uint8_t>>>& rangeProofs) const
{
    const size_t numBits = 64;
    const size_t proofLength = std::get<1>(rangeProofs.front())->size();

    std::vector<Commitment> commitments;
    std::vector<secp256k1_pedersen_commitment> secpCommitments;
    commitments.reserve(rangeProofs.size());

    std::vector<const uint8_t*> bulletproofPointers;
    bulletproofPointers.reserve(rangeProofs.size());

    std::vector<const uint8_t*> extraData;
    extraData.reserve(rangeProofs.size());

    std::vector<size_t> extraDataLen;
    extraDataLen.reserve(rangeProofs.size());

    for (const auto& rangeProof : rangeProofs)
    {
        if (!m_cache.WasAlreadyVerified(rangeProof)) {
            commitments.push_back(std::get<0>(rangeProof));
            secpCommitments.push_back(ConversionUtil(m_context).ToSecp256k1(std::get<0>(rangeProof)));
            bulletproofPointers.emplace_back(std::get<1>(rangeProof)->data());

            const std::vector<uint8_t>& extra = std::get<2>(rangeProof);
            if (!extra.empty()) {
                extraData.push_back(extra.data());
                extraDataLen.push_back(extra.size());
            } else {
                extraData.push_back(nullptr);
                extraDataLen.push_back(0);
            }
        }
    }

    if (commitments.empty()) {
        return true;
    }

    // array of generator multiplied by value in pedersen commitments (cannot be NULL)
    std::vector<secp256k1_generator> valueGenerators;
    for (size_t i = 0; i < commitments.size(); i++)
    {
        valueGenerators.push_back(secp256k1_generator_const_h);
    }

    std::vector<secp256k1_pedersen_commitment*> commitmentPointers = VectorUtil::ToPointerVec(secpCommitments);

    secp256k1_scratch_space* pScratchSpace = secp256k1_scratch_space_create(
        m_context.Read()->Get(),
        SCRATCH_SPACE_SIZE
    );
    const int result = secp256k1_bulletproof_rangeproof_verify_multi(
        m_context.Read()->Get(),
        pScratchSpace,
        m_context.Read()->GetGenerators(),
        bulletproofPointers.data(),
        commitments.size(),
        proofLength,
        NULL,
        commitmentPointers.data(),
        1,
        numBits,
        valueGenerators.data(),
        extraData.data(),
        extraDataLen.data()
    );
    secp256k1_scratch_space_destroy(pScratchSpace);

    if (result == 1) {
        for (const auto& proof_tuple : rangeProofs)
        {
            m_cache.AddToCache(proof_tuple);
        }
    }

    return result == 1;
}

RangeProof::CPtr Bulletproofs::GenerateRangeProof(
    const uint64_t amount,
    const SecretKey& key,
    const SecretKey& privateNonce,
    const SecretKey& rewindNonce,
    const ProofMessage& proofMessage)
{
    auto contextWriter = m_context.Write();
    secp256k1_context* pContext = contextWriter->Randomized();

    std::vector<uint8_t> proofBytes(RangeProof::MAX_SIZE, 0);
    size_t proofLen = RangeProof::MAX_SIZE;

    secp256k1_scratch_space* pScratchSpace = secp256k1_scratch_space_create(pContext, SCRATCH_SPACE_SIZE);

    std::vector<const uint8_t*> blindingFactors({ key.data() });
    int result = secp256k1_bulletproof_rangeproof_prove(
        pContext,
        pScratchSpace,
        contextWriter->GetGenerators(),
        &proofBytes[0],
        &proofLen,
        NULL,
        NULL,
        NULL,
        &amount,
        NULL,
        blindingFactors.data(),
        NULL,
        1,
        &secp256k1_generator_const_h,
        64,
        rewindNonce.data(),
        privateNonce.data(),
        NULL,
        0,
        proofMessage.data()
    );
    secp256k1_scratch_space_destroy(pScratchSpace);

    if (result != 1) {
        ThrowCrypto_F("secp256k1_bulletproof_rangeproof_prove failed with error: {}", result);
    }

    proofBytes.resize(proofLen);
    return std::make_shared<RangeProof>(std::move(proofBytes));
}

std::unique_ptr<RewoundProof> Bulletproofs::RewindProof(
    const Commitment& commitment,
    const RangeProof& rangeProof,
    const SecretKey& nonce) const
{
    secp256k1_pedersen_commitment secpCommitment = ConversionUtil(m_context).ToSecp256k1(commitment);

    uint64_t value;
    SecretKey blindingFactor;
    std::vector<uint8_t> message(20, 0);

    int result = secp256k1_bulletproof_rangeproof_rewind(
        m_context.Read()->Get(),
        &value,
        blindingFactor.data(),
        rangeProof.data(),
        rangeProof.size(),
        0,
        &secpCommitment,
        &secp256k1_generator_const_h,
        nonce.data(),
        NULL,
        0,
        message.data()
    );

    if (result == 1) {
        return std::make_unique<RewoundProof>(
            value, 
            std::make_unique<SecretKey>(std::move(blindingFactor)),
            ProofMessage(BigInt<20>(std::move(message)))
        );
    }

    return std::unique_ptr<RewoundProof>(nullptr);
}