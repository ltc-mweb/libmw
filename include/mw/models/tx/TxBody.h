#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/common/Logger.h>
#include <mw/models/crypto/BigInteger.h>
#include <mw/traits/Serializable.h>
#include <mw/traits/Jsonable.h>
#include <mw/models/tx/Input.h>
#include <mw/models/tx/Output.h>
#include <mw/models/tx/Kernel.h>
#include <mw/consensus/CutThrough.h>
#include <mw/crypto/Bulletproofs.h>
#include <mw/crypto/Schnorr.h>

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
        : m_inputs(std::move(inputs)), m_outputs(std::move(outputs)), m_kernels(std::move(kernels)) { }
    TxBody(const std::vector<Input>& inputs, const std::vector<Output>& outputs, const std::vector<Kernel>& kernels)
        : m_inputs(inputs), m_outputs(outputs), m_kernels(kernels) { }
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

    bool operator==(const TxBody& rhs) const noexcept
    {
        return
            m_inputs == rhs.m_inputs &&
            m_outputs == rhs.m_outputs &&
            m_kernels == rhs.m_kernels;
    }

    //
    // Getters
    //
    const std::vector<Input>& GetInputs() const noexcept { return m_inputs; }
    const std::vector<Output>& GetOutputs() const noexcept { return m_outputs; }
    const std::vector<Kernel>& GetKernels() const noexcept { return m_kernels; }

    std::vector<Kernel> GetPegInKernels() const noexcept;
    std::vector<Output> GetPegInOutputs() const noexcept;
    uint64_t GetPegInAmount() const noexcept;
    std::vector<Kernel> GetPegOutKernels() const noexcept;
    uint64_t GetTotalFee() const noexcept;

    //
    // Serialization/Deserialization
    //
    Serializer& Serialize(Serializer& serializer) const noexcept final;
    static TxBody Deserialize(Deserializer& deserializer);

    json ToJSON() const noexcept final;
    static TxBody FromJSON(const Json& json);

    void Validate() const;

private:
    // List of inputs spent by the transaction.
    std::vector<Input> m_inputs;

    // List of outputs the transaction produces.
    std::vector<Output> m_outputs;

    // List of kernels that make up this transaction.
    std::vector<Kernel> m_kernels;
};