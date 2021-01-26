#pragma once

#include <mw/common/Macros.h>
#include <mw/models/tx/Output.h>
#include <mw/models/wallet/StealthAddress.h>
#include <mw/crypto/Bulletproofs.h>
#include <mw/crypto/Random.h>

TEST_NAMESPACE

class TxOutput
{
public:
    TxOutput(const BlindingFactor& blindingFactor, const uint64_t amount, const Output& output)
        : m_blindingFactor(blindingFactor), m_amount(amount), m_output(output) { }

    static TxOutput Create(
        const EOutputFeatures features,
        const BlindingFactor& blinding_factor,
        const SecretKey& sender_privkey,
        const StealthAddress& receiver_addr,
        const uint64_t amount)
    {
        Output output = Output::Create(features, blinding_factor, sender_privkey, receiver_addr, amount);

        return TxOutput{ blinding_factor, amount, std::move(output) };
    }

    const BlindingFactor& GetBlindingFactor() const noexcept { return m_blindingFactor; }
    uint64_t GetAmount() const noexcept { return m_amount; }
    const Output& GetOutput() const noexcept { return m_output; }
    EOutputFeatures GetFeatures() const noexcept { return m_output.GetFeatures(); }
    const Commitment& GetCommitment() const noexcept { return m_output.GetCommitment(); }

private:
    BlindingFactor m_blindingFactor;
    uint64_t m_amount;
    Output m_output;
};

END_NAMESPACE