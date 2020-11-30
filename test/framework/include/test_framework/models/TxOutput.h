#pragma once

#include <mw/common/Macros.h>
#include <mw/models/tx/Output.h>
#include <mw/crypto/Random.h>

TEST_NAMESPACE

class TxOutput
{
public:
    TxOutput(const BlindingFactor& blindingFactor, const uint64_t amount, const Output& output)
        : m_blindingFactor(blindingFactor), m_amount(amount), m_output(output) { }

    static TxOutput Create(
        const EOutputFeatures features,
        const BlindingFactor& blindingFactor,
        const uint64_t amount)
    {
        Commitment commitment = Crypto::CommitBlinded(
            amount,
            blindingFactor
        );

        RangeProof::CPtr pRangeProof = Crypto::GenerateRangeProof(
            amount,
            BlindingFactor(blindingFactor).ToSecretKey(),
            SecretKey(),
            SecretKey(),
            ProofMessage(BigInt<20>()),
            OwnerData().Serialized() // TODO: Implement OwnerData
        );

        return TxOutput(
            blindingFactor,
            amount,
            Output{ features, std::move(commitment), OwnerData{}, pRangeProof }
        );
    }

    const BlindingFactor& GetBlindingFactor() const noexcept { return m_blindingFactor; }
    uint64_t GetAmount() const noexcept { return m_amount; }
    const Output& GetOutput() const noexcept { return m_output; }

private:
    BlindingFactor m_blindingFactor;
    uint64_t m_amount;
    Output m_output;
};

END_NAMESPACE