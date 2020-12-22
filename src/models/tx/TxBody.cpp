#include <mw/models/tx/TxBody.h>

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
        .Append<uint64_t>(m_inputs.size())
        .Append<uint64_t>(m_outputs.size())
        .Append<uint64_t>(m_kernels.size());

    std::for_each(
        m_inputs.cbegin(),
        m_inputs.cend(),
        [&serializer](const auto& input) { input.Serialize(serializer); }
    );
    std::for_each(
        m_outputs.cbegin(),
        m_outputs.cend(),
        [&serializer](const auto& output) { output.Serialize(serializer); }
    );
    std::for_each(
        m_kernels.cbegin(),
        m_kernels.cend(),
        [&serializer](const auto& kernel) { kernel.Serialize(serializer); }
    );

    return serializer;
}

TxBody TxBody::Deserialize(Deserializer& deserializer)
{
    const uint64_t numInputs = deserializer.Read<uint64_t>();
    const uint64_t numOutputs = deserializer.Read<uint64_t>();
    const uint64_t numKernels = deserializer.Read<uint64_t>();

    // Deserialize inputs
    std::vector<Input> inputs;
    inputs.reserve(numInputs);
    for (uint64_t i = 0; i < numInputs; i++) {
        inputs.emplace_back(Input::Deserialize(deserializer));
    }

    // Deserialize outputs
    std::vector<Output> outputs;
    outputs.reserve(numOutputs);
    for (uint64_t i = 0; i < numOutputs; i++) {
        outputs.emplace_back(Output::Deserialize(deserializer));
    }

    // Deserialize kernels
    std::vector<Kernel> kernels;
    kernels.reserve(numKernels);
    for (uint64_t i = 0; i < numKernels; i++) {
        kernels.emplace_back(Kernel::Deserialize(deserializer));
    }

    return TxBody(std::move(inputs), std::move(outputs), std::move(kernels));
}

json TxBody::ToJSON() const noexcept
{
    return json({
        {"inputs", m_inputs},
        {"outputs", m_outputs},
        {"kernels", m_kernels}
    });
}

TxBody TxBody::FromJSON(const Json& json)
{
    return TxBody{
        json.GetRequiredVec<Input>("inputs"),
        json.GetRequiredVec<Output>("outputs"),
        json.GetRequiredVec<Kernel>("kernels")
    };
}

void TxBody::Validate() const
{
    // TODO: Validate Weight
    // TODO: Verify Sorted

    CutThrough::VerifyCutThrough(m_inputs, m_outputs);

    std::vector<SignedMessage> signatures;
    std::transform(
        m_kernels.cbegin(), m_kernels.cend(),
        std::back_inserter(signatures),
        [](const Kernel& kernel) {
            PublicKey public_key = Crypto::ToPublicKey(kernel.GetCommitment());
            return SignedMessage{ kernel.GetSignatureMessage(), std::move(public_key), kernel.GetSignature() };
        }
    );

    std::transform(
        m_inputs.cbegin(), m_inputs.cend(),
        std::back_inserter(signatures),
        [](const Input& input) {
            PublicKey public_key = Crypto::ToPublicKey(input.GetCommitment());
            return SignedMessage{ InputMessage(), std::move(public_key), input.GetSignature() };
        }
    );

    std::transform(
        m_outputs.cbegin(), m_outputs.cend(),
        std::back_inserter(signatures),
        [](const Output& output) { return output.GetOwnerData().GetSignedMsg(); }
    );

    Schnorr::BatchVerify(signatures);

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