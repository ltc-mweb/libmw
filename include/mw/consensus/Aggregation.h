#pragma once

#include <mw/models/tx/Transaction.h>
#include <cassert>

static struct
{
    bool operator()(const Traits::IHashable& a, const Traits::IHashable& b) const
    {
        return a.GetHash() < b.GetHash();
    }
} SortByHash;

static struct
{
    template<typename T>
    bool operator()(const T& a, const T& b) const
    {
        return a.GetCommitment() < b.GetCommitment();
    }
} SortByCommitment;

class Aggregation
{
public:
    //
    // Aggregates multiple transactions into 1.
    //
    static mw::Transaction::CPtr Aggregate(const std::vector<mw::Transaction::CPtr>& transactions)
    {
        if (transactions.empty()) {
            return std::make_shared<mw::Transaction>();
        }

        if (transactions.size() == 1) {
            return transactions.front();
        }

        std::vector<Input> inputs;
        std::vector<Output> outputs;
        std::vector<Kernel> kernels;
        std::vector<BlindingFactor> kernel_offsets;
        std::vector<BlindingFactor> owner_offsets;
        std::vector<SignedMessage> owner_sigs;

        // collect all the inputs, outputs, kernels, and owner sigs from the txs
        for (const mw::Transaction::CPtr& pTransaction : transactions) {
            inputs.insert(
                inputs.end(),
                pTransaction->GetInputs().begin(),
                pTransaction->GetInputs().end()
            );

            outputs.insert(
                outputs.end(),
                pTransaction->GetOutputs().begin(),
                pTransaction->GetOutputs().end()
            );

            kernels.insert(
                kernels.end(),
                pTransaction->GetKernels().begin(),
                pTransaction->GetKernels().end()
            );

            owner_sigs.insert(
                owner_sigs.end(),
                pTransaction->GetOwnerSigs().begin(),
                pTransaction->GetOwnerSigs().end()
            );

            kernel_offsets.push_back(pTransaction->GetKernelOffset());
            owner_offsets.push_back(pTransaction->GetOwnerOffset());
        }

        // Perform cut-through
        // CutThrough::PerformCutThrough(inputs, outputs);
        // TODO: Prevent spending output that's created in the same transaction?

        // Sort the components
        std::sort(kernels.begin(), kernels.end(), SortByHash);
        std::sort(inputs.begin(), inputs.end(), SortByCommitment);
        std::sort(outputs.begin(), outputs.end(), SortByCommitment);
        std::sort(owner_sigs.begin(), owner_sigs.end(), SortByHash);

        // Sum the offsets up to give us an aggregate offsets for the transaction.
        BlindingFactor kernel_offset = Crypto::AddBlindingFactors(kernel_offsets);
        BlindingFactor owner_offset = Crypto::AddBlindingFactors(owner_offsets);

        // Build a new aggregate tx from the following:
        //   * cut-through inputs
        //   * cut-through outputs
        //   * full set of tx kernels
        //   * sum of all offsets
        return std::make_shared<mw::Transaction>(
            std::move(kernel_offset),
            std::move(owner_offset),
            TxBody{ std::move(inputs), std::move(outputs), std::move(kernels), std::move(owner_sigs) }
        );
    }
};