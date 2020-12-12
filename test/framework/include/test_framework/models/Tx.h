#pragma once

#include <mw/common/Macros.h>
#include <mw/models/tx/Transaction.h>
#include <mw/models/tx/PegInCoin.h>
#include <mw/crypto/Random.h>
#include <mw/crypto/Hasher.h>

#include <test_framework/models/TxOutput.h>

TEST_NAMESPACE

class Tx
{
public:
    struct Builder
    {
        BlindingFactor kernel_offset;
        BlindingFactor owner_offset;
        std::vector<Input> inputs;
        std::vector<Output> outputs;
        std::vector<Kernel> kernels;

        Builder& SetKernelOffset(const BlindingFactor& offsetIn)
        {
            kernel_offset = offsetIn;
            return *this;
        }

        Builder& SetOwnerOffset(const BlindingFactor& offsetIn)
        {
            owner_offset = offsetIn;
            return *this;
        }

        Builder& AddInput(const Input& input)
        {
            inputs.push_back(input);
            return *this;
        }

        Builder& AddOutput(const test::TxOutput& output)
        {
            outputs.push_back(output.GetOutput());
            return *this;
        }

        Builder& AddKernel(const Kernel& kernel)
        {
            kernels.push_back(kernel);
            return *this;
        }

        Builder& AddPlainKernel(const uint64_t fee, const SecretKey& excess)
        {
            Commitment excess_commitment = Crypto::CommitBlinded(0, excess);
            std::vector<uint8_t> kernel_message = Serializer()
                .Append<uint8_t>(KernelType::PLAIN_KERNEL)
                .Append<uint64_t>(fee)
                .vec();

            Signature signature = Schnorr::Sign(excess.data(), Hashed(kernel_message));
            Kernel kernel = Kernel::CreatePlain(fee, std::move(excess_commitment), std::move(signature));
            kernels.push_back(std::move(kernel));
            return *this;
        }

        Tx Build() const
        {
            return Tx(
                std::make_shared<mw::Transaction>(kernel_offset, owner_offset, TxBody(inputs, outputs, kernels))
            );
        }
    };

    Tx(const mw::Transaction::CPtr& pTransaction)
        : m_pTransaction(pTransaction) { }

    static Tx CreatePegIn(const uint64_t amount)
    {
        BlindingFactor txOffset = Random().CSPRNG<32>();

        BlindingFactor outputBF = Random().CSPRNG<32>();
        test::TxOutput output = test::TxOutput::Create(EOutputFeatures::PEGGED_IN, outputBF, amount);

        BlindingFactor kernelBF = Crypto::AddBlindingFactors({ outputBF }, { txOffset });
        Commitment kernelCommit = Crypto::CommitBlinded(0, kernelBF);

        Serializer serializer;
        serializer.Append<uint8_t>((uint8_t)KernelType::PEGIN_KERNEL);
        serializer.Append<uint64_t>(amount);

        Signature signature = Schnorr::Sign(
            kernelBF.data(),
            Hashed(serializer.vec())
        );

        Kernel kernel = Kernel::CreatePegIn(amount, std::move(kernelCommit), std::move(signature));

        return Tx::Builder().SetKernelOffset(txOffset).AddKernel(kernel).AddOutput(output).Build();
    }

    const mw::Transaction::CPtr& GetTransaction() const noexcept { return m_pTransaction; }
    const std::vector<Kernel>& GetKernels() const noexcept { return m_pTransaction->GetKernels(); }
    const std::vector<Output>& GetOutputs() const noexcept { return m_pTransaction->GetOutputs(); }

    PegInCoin GetPegInCoin() const
    {
        const auto& kernel = m_pTransaction->GetKernels().front();
        return PegInCoin(kernel.GetAmount(), kernel.GetCommitment());
    }

private:
    mw::Transaction::CPtr m_pTransaction;
};

END_NAMESPACE