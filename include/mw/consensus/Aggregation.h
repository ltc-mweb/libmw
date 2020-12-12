#pragma once

#include <mw/models/tx/Transaction.h>
#include <mw/consensus/CutThrough.h>
#include <cassert>

static struct
{
    bool operator()(const Traits::IHashable& a, const Traits::IHashable& b) const
    {
        return a.GetHash() < b.GetHash();
    }
} SortByHash;

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

        // collect all the inputs, outputs and kernels from the txs
        for (const mw::Transaction::CPtr& pTransaction : transactions) {
            for (const Input& input : pTransaction->GetInputs()) {
                inputs.push_back(input);
            }

            for (const Output& output : pTransaction->GetOutputs()) {
                outputs.push_back(output);
            }

            for (const Kernel& kernel : pTransaction->GetKernels()) {
                kernels.push_back(kernel);
            }

            kernel_offsets.push_back(pTransaction->GetKernelOffset());
            owner_offsets.push_back(pTransaction->GetOwnerOffset());
        }

        // Perform cut-through
        // CutThrough::PerformCutThrough(inputs, outputs);
        // TODO: Prevent spending output that's created in the same transaction

        // Sort the kernels.
        std::sort(kernels.begin(), kernels.end(), SortByHash);
        std::sort(inputs.begin(), inputs.end(), SortByHash);
        std::sort(outputs.begin(), outputs.end(), SortByHash);

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
            TxBody{ std::move(inputs), std::move(outputs), std::move(kernels) }
        );
    }
};