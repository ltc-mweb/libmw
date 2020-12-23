#pragma once

#include <mw/crypto/Keys.h>
#include <mw/exceptions/ValidationException.h>
#include <mw/models/crypto/BlindingFactor.h>
#include <mw/models/tx/Output.h>
#include <mw/models/tx/UTXO.h>

class OwnerValidator
{
public:
    static void ValidateOwner(
        const std::vector<UTXO::CPtr>& inputs,
        const std::vector<Output>& outputs,
        const std::vector<SignedMessage>& owner_sigs,
        const BlindingFactor& owner_offset)
    {
        std::vector<PublicKey> output_pubkeys;
        std::transform(
            outputs.cbegin(), outputs.cend(),
            std::back_inserter(output_pubkeys),
            [](const Output& output) { return output.GetOwnerData().GetSenderPubKey(); }
        );

        std::vector<PublicKey> input_pubkeys;
        std::transform(
            inputs.cbegin(), inputs.cend(),
            std::back_inserter(input_pubkeys),
            [](const UTXO::CPtr& pUTXO) { return pUTXO->GetOwnerData().GetReceiverPubKey(); }
        );

        std::transform(
            owner_sigs.cbegin(), owner_sigs.cend(),
            std::back_inserter(input_pubkeys),
            [](const SignedMessage& owner_sig) { return owner_sig.public_key; }
        );

        input_pubkeys.push_back(Crypto::CalculatePublicKey(owner_offset));

        PublicKey total_input_pubkey = Crypto::AddPublicKeys(input_pubkeys);
        PublicKey total_output_pubkey = Crypto::AddPublicKeys(output_pubkeys);

        if (total_input_pubkey != total_output_pubkey) {
            ThrowValidation(EConsensusError::OWNER_SUMS);
        }
    }
};