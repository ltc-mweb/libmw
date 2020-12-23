#include <mw/models/tx/TxBody.h>
#include <mw/exceptions/ValidationException.h>

#include <unordered_set>

std::vector<Kernel> TxBody::GetPegInKernels() const noexcept
{
    std::vector<Kernel> peggedIn;
    std::copy_if(
        m_kernels.cbegin(), m_kernels.cend(),
        std::back_inserter(peggedIn),
        [](const auto& kernel) -> bool { return kernel.IsPegIn(); }
    );

    return peggedIn;
}

std::vector<Output> TxBody::GetPegInOutputs() const noexcept
{
    std::vector<Output> peggedIn;
    std::copy_if(
        m_outputs.cbegin(), m_outputs.cend(),
        std::back_inserter(peggedIn),
        [](const Output& output) -> bool { return output.IsPeggedIn(); }
    );

    return peggedIn;
}

uint64_t TxBody::GetPegInAmount() const noexcept
{
    return std::accumulate(
        m_kernels.cbegin(), m_kernels.cend(), (uint64_t)0,
        [](const uint64_t sum, const auto& kernel) noexcept { return sum + kernel.GetPeggedIn(); }
    );
}

std::vector<Kernel> TxBody::GetPegOutKernels() const noexcept
{
    std::vector<Kernel> peggedOut;
    std::copy_if(
        m_kernels.cbegin(), m_kernels.cend(),
        std::back_inserter(peggedOut),
        [](const auto& kernel) -> bool { return kernel.IsPegOut(); }
    );

    return peggedOut;
}

uint64_t TxBody::GetTotalFee() const noexcept
{
    return std::accumulate(
        m_kernels.cbegin(), m_kernels.cend(), (uint64_t)0,
        [](const uint64_t sum, const auto& kernel) noexcept { return sum + kernel.GetFee(); }
    );
}

Serializer& TxBody::Serialize(Serializer& serializer) const noexcept
{
    serializer
        .Append<uint32_t>((uint32_t)m_inputs.size())
        .Append<uint32_t>((uint32_t)m_outputs.size())
        .Append<uint32_t>((uint32_t)m_kernels.size())
        .Append<uint32_t>((uint32_t)m_ownerSigs.size());

    std::for_each(
        m_inputs.cbegin(), m_inputs.cend(),
        [&serializer](const auto& input) { input.Serialize(serializer); }
    );
    std::for_each(
        m_outputs.cbegin(), m_outputs.cend(),
        [&serializer](const auto& output) { output.Serialize(serializer); }
    );
    std::for_each(
        m_kernels.cbegin(), m_kernels.cend(),
        [&serializer](const auto& kernel) { kernel.Serialize(serializer); }
    );
    std::for_each(
        m_ownerSigs.cbegin(), m_ownerSigs.cend(),
        [&serializer](const auto& owner_sig) { owner_sig.Serialize(serializer); }
    );

    return serializer;
}

TxBody TxBody::Deserialize(Deserializer& deserializer)
{
    const uint32_t numInputs = deserializer.Read<uint32_t>();
    const uint32_t numOutputs = deserializer.Read<uint32_t>();
    const uint32_t numKernels = deserializer.Read<uint32_t>();
    const uint32_t numOwnerSigs = deserializer.Read<uint32_t>();

    // Deserialize inputs
    std::vector<Input> inputs;
    inputs.reserve(numInputs);
    for (uint32_t i = 0; i < numInputs; i++) {
        inputs.emplace_back(Input::Deserialize(deserializer));
    }

    // Deserialize outputs
    std::vector<Output> outputs;
    outputs.reserve(numOutputs);
    for (uint32_t i = 0; i < numOutputs; i++) {
        outputs.emplace_back(Output::Deserialize(deserializer));
    }

    // Deserialize kernels
    std::vector<Kernel> kernels;
    kernels.reserve(numKernels);
    for (uint32_t i = 0; i < numKernels; i++) {
        kernels.emplace_back(Kernel::Deserialize(deserializer));
    }

    // Deserialize owner sigs
    std::vector<SignedMessage> owner_sigs;
    owner_sigs.reserve(numOwnerSigs);
    for (uint32_t i = 0; i < numOwnerSigs; i++) {
        owner_sigs.emplace_back(SignedMessage::Deserialize(deserializer));
    }

    return TxBody(std::move(inputs), std::move(outputs), std::move(kernels), std::move(owner_sigs));
}

void TxBody::Validate() const
{
    // TODO: Validate Weight
    // TODO: Verify Sorted

    // Verify no duplicate inputs
    std::unordered_set<Commitment> input_commits;
    std::transform(
        m_inputs.cbegin(), m_inputs.cend(),
        std::inserter(input_commits, input_commits.end()),
        [](const Input& input) { return input.GetCommitment(); }
    );

    if (input_commits.size() != m_inputs.size()) {
        ThrowValidation(EConsensusError::DUPLICATE_COMMITS);
    }

    // Verify no duplicate outputs
    std::unordered_set<Commitment> output_commits;
    std::transform(
        m_outputs.cbegin(), m_outputs.cend(),
        std::inserter(output_commits, output_commits.end()),
        [](const Output& output) { return output.GetCommitment(); }
    );

    if (output_commits.size() != m_outputs.size()) {
        ThrowValidation(EConsensusError::DUPLICATE_COMMITS);
    }

    // TODO: Verify kernel exists with matching hash for each owner sig

    //
    // Verify all signatures
    //
    std::vector<SignedMessage> signatures;
    std::transform(
        m_kernels.cbegin(), m_kernels.cend(),
        std::back_inserter(signatures),
        [](const Kernel& kernel) {
            PublicKey public_key = Crypto::ToPublicKey(kernel.GetCommitment());
            return SignedMessage{ kernel.GetSignatureMessage(), public_key, kernel.GetSignature() };
        }
    );

    std::transform(
        m_inputs.cbegin(), m_inputs.cend(),
        std::back_inserter(signatures),
        [](const Input& input) {
            return SignedMessage{ InputMessage(), input.GetPubKey(), input.GetSignature() };
        }
    );

    std::transform(
        m_outputs.cbegin(), m_outputs.cend(),
        std::back_inserter(signatures),
        [](const Output& output) { return output.GetOwnerData().GetSignedMsg(); }
    );

    signatures.insert(signatures.end(), m_ownerSigs.begin(), m_ownerSigs.end());

    if (!Schnorr::BatchVerify(signatures)) {
        ThrowValidation(EConsensusError::INVALID_SIG);
    }

    //
    // Verify RangeProofs
    //
    std::vector<ProofData> rangeProofs;
    std::transform(
        m_outputs.cbegin(), m_outputs.cend(),
        std::back_inserter(rangeProofs),
        [](const Output& output) {
            return ProofData{ output.GetCommitment(), output.GetRangeProof(), output.GetOwnerData().Serialized() };
        }
    );
    if (!Bulletproofs::BatchVerify(rangeProofs)) {
        ThrowValidation(EConsensusError::BULLETPROOF);
    }
}