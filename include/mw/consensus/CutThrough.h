#pragma once

#include <mw/models/tx/Input.h>
#include <mw/models/tx/Output.h>
#include <mw/exceptions/ValidationException.h>
#include <unordered_set>

class CutThrough
{
public:
    static void PerformCutThrough(std::vector<Input>& inputs, std::vector<Output>& outputs)
    {
        std::unordered_set<Commitment> inputCommitments;
        for (const Input& input : inputs) {
            inputCommitments.insert(input.GetCommitment());
        }

        std::unordered_set<Commitment> outputCommitments;
        for (const Output& output : outputs) {
            outputCommitments.insert(output.GetCommitment());
        }

        auto inputIter = inputs.begin();
        while (inputIter != inputs.end()) {
            if (outputCommitments.find(inputIter->GetCommitment()) != outputCommitments.end()) {
                inputIter = inputs.erase(inputIter);
            } else {
                ++inputIter;
            }
        }

        auto outputIter = outputs.begin();
        while (outputIter != outputs.end()) {
            if (inputCommitments.find(outputIter->GetCommitment()) != inputCommitments.end()) {
                outputIter = outputs.erase(outputIter);
            } else {
                ++outputIter;
            }
        }
    }

    static void VerifyCutThrough(const std::vector<Input>& inputs, const std::vector<Output>& outputs)
    {
        // Create set with output commitments
        std::unordered_set<Commitment> commitments;
        std::transform(
            outputs.cbegin(), outputs.cend(),
            std::inserter(commitments, commitments.end()),
            [](const Output& output) { return output.GetCommitment(); }
        );

        // TODO: Confirm we don't allow duplicates within a single block?
        if (commitments.size() != outputs.size()) {
            ThrowValidation(EConsensusError::CUT_THROUGH);
        }

        // Verify none of the input commitments are in the commitments set
        const bool invalid = std::any_of(
            inputs.cbegin(), inputs.cend(),
            [&commitments](const Input& input) { return commitments.find(input.GetCommitment()) != commitments.cend(); }
        );
        if (invalid) {
            ThrowValidation(EConsensusError::CUT_THROUGH);
        }
    }
};