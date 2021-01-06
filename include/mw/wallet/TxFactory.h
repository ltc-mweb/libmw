#pragma once

#include <mw/models/tx/Transaction.h>

class TxFactory
{
public:
    static mw::Transaction::CPtr CreateTx(
        BlindingFactor kernel_offset,
        BlindingFactor owner_offset,
        std::vector<Input> inputs,
        std::vector<Output> outputs,
        std::vector<Kernel> kernels,
        std::vector<SignedMessage> owner_sigs)
    {
        std::sort(inputs.begin(), inputs.end(), SortByCommitment);
        std::sort(outputs.begin(), outputs.end(), SortByCommitment);
        std::sort(kernels.begin(), kernels.end(), SortByHash);
        std::sort(owner_sigs.begin(), owner_sigs.end(), SortByHash);

        return std::make_shared<mw::Transaction>(
            std::move(kernel_offset),
            std::move(owner_offset),
            TxBody{
                std::move(inputs),
                std::move(outputs),
                std::move(kernels),
                std::move(owner_sigs)
            }
        );
    }
};