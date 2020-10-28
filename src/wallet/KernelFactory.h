#pragma once

#include <mw/crypto/Crypto.h>
#include <mw/models/tx/Kernel.h>
#include <mw/models/crypto/BlindingFactor.h>

class KernelFactory
{
public:
    static Kernel CreatePlainKernel(const BlindingFactor& kernel_blind, const uint64_t fee)
    {
        Commitment kernel_commit = Crypto::CommitBlinded(0, kernel_blind);

        std::vector<uint8_t> sig_message = Serializer()
            .Append<uint8_t>(KernelType::PLAIN_KERNEL)
            .Append<uint64_t>(fee)
            .vec();
        Signature sig = Crypto::BuildSignature(BlindingFactor(kernel_blind).ToSecretKey(), Hashed(sig_message));
        return Kernel::CreatePlain(
            fee,
            std::move(kernel_commit),
            std::move(sig)
        );
    }

    static Kernel CreatePegInKernel(const BlindingFactor& kernel_blind, const uint64_t amount)
    {
        Commitment kernel_commit = Crypto::CommitBlinded(0, kernel_blind);
        std::vector<uint8_t> sig_message = Serializer()
            .Append<uint8_t>(KernelType::PEGIN_KERNEL)
            .Append<uint64_t>(amount)
            .vec();
        Signature sig = Crypto::BuildSignature(BlindingFactor(kernel_blind).ToSecretKey(), Hashed(sig_message));
        return Kernel::CreatePegIn(amount, std::move(kernel_commit), std::move(sig));
    }

    static Kernel CreatePegOutKernel(
        const BlindingFactor& kernel_blind,
        const uint64_t amount,
        const uint64_t fee,
        const Bech32Address& address)
    {
        Commitment kernel_commit = Crypto::CommitBlinded(0, kernel_blind);

        std::vector<uint8_t> sig_message = Serializer()
            .Append<uint8_t>(KernelType::PEGOUT_KERNEL)
            .Append<uint64_t>(fee)
            .Append<uint64_t>(amount)
            .Append(address)
            .vec();
        Signature sig = Crypto::BuildSignature(BlindingFactor(kernel_blind).ToSecretKey(), Hashed(sig_message));
        return Kernel::CreatePegOut(
            amount,
            fee,
            Bech32Address(address),
            std::move(kernel_commit),
            std::move(sig)
        );
    }
};