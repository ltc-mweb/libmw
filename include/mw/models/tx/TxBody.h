#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/models/crypto/BigInteger.h>
#include <mw/traits/Serializable.h>
#include <mw/traits/Jsonable.h>
#include <mw/models/tx/Input.h>
#include <mw/models/tx/Output.h>
#include <mw/models/tx/Kernel.h>
#include <mw/consensus/CutThrough.h>
#include <mw/consensus/KernelSignatureValidator.h>

#include <memory>
#include <vector>

////////////////////////////////////////
// TRANSACTION BODY - Container for all inputs, outputs, and kernels in a transaction or block.
////////////////////////////////////////
class TxBody :
    public Traits::ISerializable,
    public Traits::IJsonable
{
public:
    using CPtr = std::shared_ptr<const TxBody>;

    //
    // Constructors
    //
    TxBody(std::vector<Input>&& inputs, std::vector<Output>&& outputs, std::vector<Kernel>&& kernels)
        : m_inputs(std::move(inputs)), m_outputs(std::move(outputs)), m_kernels(std::move(kernels))
    {

    }
    TxBody(const TxBody& other) = default;
    TxBody(TxBody&& other) noexcept = default;
    TxBody() = default;

    //
    // Destructor
    //
    virtual ~TxBody() = default;

    //
    // Operators
    //
    TxBody& operator=(const TxBody& other) = default;
    TxBody& operator=(TxBody&& other) noexcept = default;

    //
    // Getters
    //
    const std::vector<Input>& GetInputs() const noexcept { return m_inputs; }
    const std::vector<Output>& GetOutputs() const noexcept { return m_outputs; }
    const std::vector<Kernel>& GetKernels() const noexcept { return m_kernels; }

    //
    // Serialization/Deserialization
    //
    Serializer& Serialize(Serializer& serializer) const noexcept final
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

    static TxBody Deserialize(Deserializer& deserializer)
    {
        const uint64_t numInputs = deserializer.Read<uint64_t>();
        const uint64_t numOutputs = deserializer.Read<uint64_t>();
        const uint64_t numKernels = deserializer.Read<uint64_t>();

        // Deserialize outputs
        std::vector<Input> inputs;
        inputs.reserve(numInputs);
        for (uint64_t i = 0; i < numInputs; i++)
        {
            inputs.emplace_back(Input::Deserialize(deserializer));
        }

        // Deserialize outputs
        std::vector<Output> outputs;
        outputs.reserve(numOutputs);
        for (uint64_t i = 0; i < numOutputs; i++)
        {
            outputs.emplace_back(Output::Deserialize(deserializer));
        }

        // Deserialize kernels
        std::vector<Kernel> kernels;
        kernels.reserve(numKernels);
        for (uint64_t i = 0; i < numKernels; i++)
        {
            kernels.emplace_back(Kernel::Deserialize(deserializer));
        }

        return TxBody(std::move(inputs), std::move(outputs), std::move(kernels));
    }

    json ToJSON() const noexcept final
    {
        return json({
            {"inputs", m_inputs},
            {"outputs", m_outputs},
            {"kernels", m_kernels}
        });
    }

    static TxBody FromJSON(const Json&)
    {
        // TODO: Implement
    }

    void Validate() const
    {
        CutThrough::VerifyCutThrough(m_inputs, m_outputs);
        KernelSignatureValidator::VerifyKernelSignatures(m_kernels);

        // TODO: Validate Weight
        // TODO: Verify Sorted

        //
        // Verify RangeProofs
        //
        std::vector<std::pair<Commitment, RangeProof::CPtr>> rangeProofs;
        std::transform(
            m_outputs.cbegin(), m_outputs.cend(),
            std::back_inserter(rangeProofs),
            [](const Output& output) { return std::make_pair(output.GetCommitment(), output.GetRangeProof()); }
        );
        if (!Crypto::VerifyRangeProofs(rangeProofs))
        {
            ThrowValidation(EConsensusError::BULLETPROOF);
        }
    }

private:
    // List of inputs spent by the transaction.
    std::vector<Input> m_inputs; // TODO: Use set or multiset?

    // List of outputs the transaction produces.
    std::vector<Output> m_outputs;

    // List of kernels that make up this transaction.
    std::vector<Kernel> m_kernels;
};