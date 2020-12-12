#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/common/Macros.h>
#include <mw/crypto/Crypto.h>
#include <mw/models/crypto/Hash.h>
#include <mw/models/crypto/BigInteger.h>
#include <mw/models/crypto/BlindingFactor.h>
#include <mw/models/tx/TxBody.h>
#include <mw/traits/Printable.h>
#include <mw/traits/Serializable.h>
#include <mw/traits/Hashable.h>
#include <mw/traits/Jsonable.h>

#include <memory>
#include <vector>

MW_NAMESPACE

////////////////////////////////////////
// TRANSACTION - Represents a transaction or merged transactions before they've been included in a block.
////////////////////////////////////////
class Transaction :
    public Traits::IPrintable,
    public Traits::ISerializable,
    public Traits::IHashable,
    public Traits::IJsonable
{
public:
    using CPtr = std::shared_ptr<const Transaction>;

    //
    // Constructors
    //
    Transaction(BlindingFactor&& kernel_offset, BlindingFactor&& owner_offset, TxBody&& body)
        : m_kernelOffset(std::move(kernel_offset)), m_ownerOffset(std::move(owner_offset)), m_body(std::move(body))
    {
        m_hash = Hashed(*this);
    }
    Transaction(const BlindingFactor& kernel_offset, const BlindingFactor& owner_offset, const TxBody& body)
        : Transaction(BlindingFactor(kernel_offset), BlindingFactor(owner_offset), TxBody(body)) { }
    Transaction(const Transaction& transaction) = default;
    Transaction(Transaction&& transaction) noexcept = default;
    Transaction() = default;

    //
    // Destructor
    //
    virtual ~Transaction() = default;

    //
    // Operators
    //
    Transaction& operator=(const Transaction& transaction) = default;
    Transaction& operator=(Transaction&& transaction) noexcept = default;
    bool operator<(const Transaction& transaction) const noexcept { return GetHash() < transaction.GetHash(); }
    bool operator==(const Transaction& transaction) const noexcept { return GetHash() == transaction.GetHash(); }
    bool operator!=(const Transaction& transaction) const noexcept { return GetHash() != transaction.GetHash(); }

    //
    // Getters
    //
    const BlindingFactor& GetKernelOffset() const noexcept { return m_kernelOffset; }
    const BlindingFactor& GetOwnerOffset() const noexcept { return m_ownerOffset; }
    const TxBody& GetBody() const noexcept { return m_body; }
    const std::vector<Input>& GetInputs() const noexcept { return m_body.GetInputs(); }
    const std::vector<Output>& GetOutputs() const noexcept { return m_body.GetOutputs(); }
    const std::vector<Kernel>& GetKernels() const noexcept { return m_body.GetKernels(); }
    uint64_t GetTotalFee() const noexcept { return m_body.GetTotalFee(); }

    std::vector<Kernel> GetPegInKernels() const noexcept { return m_body.GetPegInKernels(); }
    std::vector<Output> GetPegInOutputs() const noexcept { return m_body.GetPegInOutputs(); }
    uint64_t GetPegInAmount() const noexcept { return m_body.GetPegInAmount(); }
    std::vector<Kernel> GetPegOutKernels() const noexcept { return m_body.GetPegOutKernels(); }

    //
    // Serialization/Deserialization
    //
    Serializer& Serialize(Serializer& serializer) const noexcept final
    {
        return serializer
            .Append(m_kernelOffset)
            .Append(m_ownerOffset)
            .Append(m_body);
    }

    static Transaction Deserialize(Deserializer& deserializer)
    {
        BlindingFactor kernel_offset = BlindingFactor::Deserialize(deserializer);
        BlindingFactor owner_offset = BlindingFactor::Deserialize(deserializer);
        TxBody body = TxBody::Deserialize(deserializer);
        return Transaction(std::move(kernel_offset), std::move(owner_offset), std::move(body));
    }

    json ToJSON() const noexcept final
    {
        return json({
            {"kernel_offset", m_kernelOffset.ToHex()},
            {"owner_offset", m_ownerOffset.ToHex()},
            {"body", m_body.ToJSON()}
        });
    }

    static Transaction FromJSON(const Json& json)
    {
        return Transaction(
            BlindingFactor::FromHex(json.GetRequired<std::string>("kernel_offset")),
            BlindingFactor::FromHex(json.GetRequired<std::string>("owner_offset")),
            json.GetRequired<TxBody>("body")
        );
    }

    //
    // Traits
    //
    std::string Format() const final { return GetHash().Format(); }
    mw::Hash GetHash() const noexcept final { return m_hash; }

private:
    // The kernel "offset" k2 excess is k1G after splitting the key k = k1 + k2.
    BlindingFactor m_kernelOffset;

    BlindingFactor m_ownerOffset;

    // The transaction body.
    TxBody m_body;

    mutable mw::Hash m_hash;
};

END_NAMESPACE