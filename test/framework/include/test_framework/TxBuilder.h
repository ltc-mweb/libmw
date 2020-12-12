#pragma once

#include <test_framework/models/Tx.h>

TEST_NAMESPACE

//
// Builds transactions for use with automated tests using random blinding factors.
// To make the transaction valid, TxBuilder keeps track of all blinding factors used,
// and adjusts the tx offset to make a valid transaction.
//
struct TxBuilder
{
    TxBuilder()
        : m_built{ false }, m_amount{}, m_kernelOffset{}, m_ownerOffset{}, m_inputs{}, m_outputs{}, m_kernels{}
    {

    }

    TxBuilder& AddInput(const uint64_t amount, const EOutputFeatures features = EOutputFeatures::DEFAULT_OUTPUT)
    {
        assert(!m_built);

        BlindingFactor input_bf = Random::CSPRNG<32>();
        m_kernelOffset = Crypto::AddBlindingFactors({ m_kernelOffset }, { input_bf });
        mw::Hash mweb_hash = Hashed(std::vector<uint8_t>{'M', 'W', 'E', 'B'}); // TODO: Determine actual message
        Signature sig = Schnorr::Sign(input_bf.data(), mweb_hash);
        m_inputs.push_back(Input{ Crypto::CommitBlinded(amount, input_bf), std::move(sig) });
        m_amount += (int64_t)amount;
        return *this;
    }

    TxBuilder& AddOutput(const uint64_t amount, const EOutputFeatures features = EOutputFeatures::DEFAULT_OUTPUT)
    {
        assert(!m_built);

        BlindingFactor output_bf = Random::CSPRNG<32>();
        m_kernelOffset = Crypto::AddBlindingFactors({ m_kernelOffset, output_bf });

        RangeProof::CPtr pRangeProof = Crypto::GenerateRangeProof(
            amount,
            BlindingFactor(output_bf).ToSecretKey(),
            SecretKey(),
            SecretKey(),
            ProofMessage(BigInt<20>()),
            OwnerData().Serialized() // TODO: Implement OwnerData
        );

        m_outputs.push_back(Output{ features, Crypto::CommitBlinded(amount, output_bf), {}, pRangeProof });
        m_amount -= (int64_t)amount;
        return *this;
    }

    TxBuilder& AddPlainKernel(const uint64_t fee)
    {
        assert(!m_built);

        SecretKey kernel_excess = Random::CSPRNG<32>();
        m_kernelOffset = Crypto::AddBlindingFactors({ m_kernelOffset }, { kernel_excess });

        Commitment excess_commitment = Crypto::CommitBlinded(0, kernel_excess);
        std::vector<uint8_t> kernel_message = Serializer()
            .Append<uint8_t>(KernelType::PLAIN_KERNEL)
            .Append<uint64_t>(fee)
            .vec();

        Signature signature = Crypto::BuildSignature(kernel_excess, Hashed(kernel_message));
        Kernel kernel = Kernel::CreatePlain(fee, std::move(excess_commitment), std::move(signature));

        m_kernels.push_back(std::move(kernel));
        m_amount -= (int64_t)fee;
        return *this;
    }

    TxBuilder& AddPeginKernel(const uint64_t amount)
    {
        assert(!m_built);

        SecretKey kernel_excess = Random::CSPRNG<32>();
        m_kernelOffset = Crypto::AddBlindingFactors({ m_kernelOffset }, { kernel_excess });

        Commitment excess_commitment = Crypto::CommitBlinded(0, kernel_excess);
        std::vector<uint8_t> kernel_message = Serializer()
            .Append<uint8_t>(KernelType::PEGIN_KERNEL)
            .Append<uint64_t>(amount)
            .vec();

        Signature signature = Crypto::BuildSignature(kernel_excess, Hashed(kernel_message));
        Kernel kernel = Kernel::CreatePegIn(amount, std::move(excess_commitment), std::move(signature));

        m_kernels.push_back(std::move(kernel));
        m_amount += amount;
        return *this;
    }

    TxBuilder& AddPegoutKernel(const uint64_t amount, const uint64_t fee)
    {
        assert(!m_built);

        SecretKey kernel_excess = Random::CSPRNG<32>();
        m_kernelOffset = Crypto::AddBlindingFactors({ m_kernelOffset }, { kernel_excess });
        Bech32Address ltc_address("hrp", Random::CSPRNG<32>().vec());

        Commitment excess_commitment = Crypto::CommitBlinded(0, kernel_excess);
        std::vector<uint8_t> kernel_message = Serializer()
            .Append<uint8_t>(KernelType::PEGOUT_KERNEL)
            .Append<uint64_t>(fee)
            .Append<uint64_t>(amount)
            .Append(ltc_address)
            .vec();

        Signature signature = Crypto::BuildSignature(kernel_excess, Hashed(kernel_message));
        Kernel kernel = Kernel::CreatePegOut(amount, fee, std::move(ltc_address), std::move(excess_commitment), std::move(signature));

        m_kernels.push_back(std::move(kernel));
        m_amount -= amount + fee;
        return *this;
    }

    mw::Transaction::CPtr Build()
    {
        assert(m_amount == 0);
        assert(!m_built);

        m_built = true;
        return std::make_shared<mw::Transaction>(m_kernelOffset, m_ownerOffset, TxBody{m_inputs, m_outputs, m_kernels});
    }

private:
    bool m_built;

    int64_t m_amount;
    BlindingFactor m_kernelOffset;
    BlindingFactor m_ownerOffset;

    std::vector<Input> m_inputs;
    std::vector<Output> m_outputs;
    std::vector<Kernel> m_kernels;
};

END_NAMESPACE