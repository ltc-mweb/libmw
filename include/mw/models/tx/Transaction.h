#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

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
    Transaction(BlindingFactor&& offset, TxBody&& transactionBody)
        : m_offset(std::move(offset)), m_body(std::move(transactionBody))
    {
        Serializer serializer;
        Serialize(serializer);

        m_hash = Crypto::Blake2b(serializer.vec());
    }

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
    const BlindingFactor& GetOffset() const noexcept { return m_offset; }
    const TxBody& GetBody() const noexcept { return m_body; }
    const std::vector<Input>& GetInputs() const noexcept { return m_body.GetInputs(); }
    const std::vector<Output>& GetOutputs() const noexcept { return m_body.GetOutputs(); }
    const std::vector<Kernel>& GetKernels() const noexcept { return m_body.GetKernels(); }

    //
    // Serialization/Deserialization
    //
    Serializer& Serialize(Serializer& serializer) const noexcept final
    {
        return serializer
            .Append(m_offset)
            .Append(m_body);
    }

    static Transaction Deserialize(Deserializer& deserializer)
    {
        BlindingFactor offset = BlindingFactor::Deserialize(deserializer);
        TxBody transactionBody = TxBody::Deserialize(deserializer);
        return Transaction(std::move(offset), std::move(transactionBody));
    }

    json ToJSON() const noexcept final
    {
        return json({
            {"offset", m_offset.ToHex()},
            {"body", m_body.ToJSON()}
        });
    }

    static Transaction FromJSON(const Json& json)
    {
        return Transaction(
            BlindingFactor::FromHex(json.GetRequired<std::string>("offset")),
            json.GetRequired<TxBody>("body")
        );
    }

    //
    // Traits
    //
    std::string Format() const final { return GetHash().Format(); }
    Hash GetHash() const noexcept final { return m_hash; }

private:
    // The kernel "offset" k2 excess is k1G after splitting the key k = k1 + k2.
    BlindingFactor m_offset;

    // The transaction body.
    TxBody m_body;

    mutable Hash m_hash;
};