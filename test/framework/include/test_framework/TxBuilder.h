#pragma once

#include <test_framework/models/Tx.h>

#include <mw/crypto/Blinds.h>
#include <mw/crypto/Bulletproofs.h>
#include <mw/models/wallet/StealthAddress.h>

TEST_NAMESPACE

//
// Builds transactions for use with automated tests using random blinding factors.
// To make the transaction valid, TxBuilder keeps track of all blinding factors used,
// and adjusts the tx offset to make a valid transaction.
//
class TxBuilder
{
public:
    TxBuilder();

    TxBuilder& AddInput(
        const uint64_t amount,
        const EOutputFeatures features = EOutputFeatures::DEFAULT_OUTPUT
    );
    TxBuilder& AddInput(
        const uint64_t amount,
        const SecretKey& privkey,
        const EOutputFeatures features
    );

    TxBuilder& AddOutput(
        const uint64_t amount,
        const EOutputFeatures features = EOutputFeatures::DEFAULT_OUTPUT
    );
    TxBuilder& AddOutput(
        const uint64_t amount,
        const SecretKey& sender_privkey,
        const StealthAddress& receiver_addr,
        const EOutputFeatures features
    );

    TxBuilder& AddPlainKernel(const uint64_t fee, const bool add_owner_sig = false);
    TxBuilder& AddPeginKernel(const uint64_t amount, const bool add_owner_sig = false);
    TxBuilder& AddPegoutKernel(const uint64_t amount, const uint64_t fee, const bool add_owner_sig = false);

    mw::Transaction::CPtr Build();

private:
    bool m_built;

    int64_t m_amount;
    Blinds m_kernelOffset;
    Blinds m_ownerOffset;

    std::vector<Input> m_inputs;
    std::vector<Output> m_outputs;
    std::vector<Kernel> m_kernels;
    std::vector<SignedMessage> m_ownerSigs;
};

END_NAMESPACE