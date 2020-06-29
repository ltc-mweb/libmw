#pragma once

#include <mw/common/Logger.h>
#include <mw/crypto/Crypto.h>
#include <mw/models/tx/Kernel.h>
#include <mw/exceptions/ValidationException.h>

class KernelSignatureValidator
{
public:
    // Verify the tx kernels.
    static void VerifyKernelSignatures(const std::vector<Kernel>& kernels)
    {
        std::vector<const Commitment*> commitments;
        commitments.reserve(kernels.size());

        std::vector<const Signature*> signatures;
        signatures.reserve(kernels.size());

        std::vector<mw::Hash> msgs;
        msgs.reserve(kernels.size());

        std::vector<const mw::Hash*> messages;
        messages.reserve(kernels.size());

        // Verify the transaction proof validity. Entails handling the commitment as a public key and checking the signature verifies with the fee as message.
        for (size_t i = 0; i < kernels.size(); i++)
        {
            const Kernel& kernel = kernels[i];
            commitments.push_back(&kernel.GetExcess());
            signatures.push_back(&kernel.GetSignature());
            msgs.emplace_back(kernel.GetSignatureMessage());
            messages.push_back(&msgs[i]);
        }

        if (!Crypto::VerifyKernelSignatures(signatures, commitments, messages))
        {
            ThrowValidation(EConsensusError::KERNEL_SIG);
        }
    }
};