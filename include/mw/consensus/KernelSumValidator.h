#pragma once

#include <mw/exceptions/ValidationException.h>
#include <mw/crypto/Crypto.h>
#include <mw/models/tx/TxBody.h>
#include <mw/models/tx/Transaction.h>
#include <mw/models/tx/UTXO.h>
#include <mw/common/Logger.h>
#include <cstdlib>

class KernelSumValidator
{
public:
    // Makes sure the sums of all utxo commitments minus the total supply
    // equals the sum of all kernel excesses and the total offset.
    // This is to be used only when validating the entire state.
    //
    // Throws a ValidationException if the utxo sum != kernel sum.
    static void ValidateState(
        const std::vector<Commitment>& utxo_commitments,
        const std::vector<Kernel>& kernels,
        const BlindingFactor& total_offset)
    {
        // Sum all utxo commitments - expected supply.
        uint64_t total_mw_supply = 0;
        for (const Kernel& kernel : kernels)
        {
            if (kernel.IsPegIn()) {
                total_mw_supply += kernel.GetAmount();
            } else if (kernel.IsPegOut()) {
                if (kernel.GetAmount() > total_mw_supply) {
                    ThrowValidation(EConsensusError::BLOCK_SUMS);
                }

                total_mw_supply -= kernel.GetAmount();
            }

            total_mw_supply -= kernel.GetFee();
        }

        Commitment total_utxo_commitment = Crypto::AddCommitments(
            utxo_commitments,
            { Crypto::CommitTransparent(total_mw_supply) }
        );

        // Sum the kernel excesses accounting for the kernel offset.
        std::vector<Commitment> kernel_excess_commitments;
        std::transform(
            kernels.cbegin(), kernels.cend(),
            std::back_inserter(kernel_excess_commitments),
            [](const Kernel& kernel) { return kernel.GetCommitment(); }
        );

        if (!total_offset.IsZero()) {
            kernel_excess_commitments.push_back(Crypto::CommitBlinded((uint64_t)0, total_offset));
        }

        Commitment total_excess_commitment =  Crypto::AddCommitments(kernel_excess_commitments);

        if (total_utxo_commitment != total_excess_commitment) {
            LOG_ERROR_F(
                "UTXO sum {} does not match kernel excess sum {}.",
                total_utxo_commitment,
                total_excess_commitment
            );
            ThrowValidation(EConsensusError::BLOCK_SUMS);
        }
    }

    static void ValidateForBlock(
        const TxBody& body,
        const BlindingFactor& total_offset,
        const BlindingFactor& prev_total_offset)
    {
        BlindingFactor block_offset = total_offset;
        if (!prev_total_offset.IsZero()) {
            block_offset = Crypto::AddBlindingFactors({ block_offset }, { prev_total_offset });
        }

        ValidateIncremental(body, block_offset);
    }

    static void ValidateForTx(const mw::Transaction& tx)
    {
        ValidateIncremental(tx.GetBody(), tx.GetKernelOffset());
    }

private:
    static void ValidateIncremental(
        const TxBody& body,
        const BlindingFactor& offset)
    {
        std::vector<Commitment> input_commitments;
        std::transform(
            body.GetInputs().cbegin(), body.GetInputs().cend(),
            std::back_inserter(input_commitments),
            [](const Input& input) { return input.GetCommitment(); }
        );

        std::vector<Commitment> output_commitments;
        std::transform(
            body.GetOutputs().cbegin(), body.GetOutputs().cend(),
            std::back_inserter(output_commitments),
            [](const Output& output) { return output.GetCommitment(); }
        );

        int64_t coins_added = 0;
        for (const Kernel& kernel : body.GetKernels())
        {
            if (kernel.IsPegIn()) {
                coins_added += (int64_t)kernel.GetAmount();
            }
            else if (kernel.IsPegOut()) {
                coins_added -= (int64_t)kernel.GetAmount();
            }

            coins_added -= (int64_t)kernel.GetFee();
        }

        if (coins_added > 0) {
            input_commitments.push_back(Crypto::CommitTransparent(coins_added));
        } else if (coins_added < 0) {
            output_commitments.push_back(Crypto::CommitTransparent(std::abs(coins_added)));
        }

        Commitment sum_utxo_commitment;
        if (!input_commitments.empty() || !output_commitments.empty()) {
            sum_utxo_commitment = Crypto::AddCommitments(output_commitments, input_commitments);
        }

        std::vector<Commitment> kernel_excess_commitments;
        std::transform(
            body.GetKernels().cbegin(), body.GetKernels().cend(),
            std::back_inserter(kernel_excess_commitments),
            [](const Kernel& kernel) { return kernel.GetExcess(); }
        );

        if (!offset.IsZero()) {
            kernel_excess_commitments.push_back(Crypto::CommitBlinded((uint64_t)0, offset));
        }

        Commitment sum_excess_commitment;
        if (!kernel_excess_commitments.empty()) {
            sum_excess_commitment = Crypto::AddCommitments(kernel_excess_commitments);
        }

        if (sum_utxo_commitment != sum_excess_commitment) {
            LOG_ERROR_F(
                "UTXO sum {} does not match kernel excess sum {}.",
                sum_utxo_commitment,
                sum_excess_commitment
            );
            ThrowValidation(EConsensusError::BLOCK_SUMS);
        }
    }
};