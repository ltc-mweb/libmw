#pragma once

#include <mw/common/Macros.h>
#include <mw/crypto/Crypto.h>
#include <mw/crypto/Hasher.h>
#include <mw/traits/Committed.h>
#include <mw/traits/Hashable.h>
#include <mw/traits/Serializable.h>
#include <mw/traits/Printable.h>
#include <mw/models/crypto/Bech32Address.h>
#include <mw/models/crypto/BlindingFactor.h>
#include <mw/models/crypto/Signature.h>
#include <mw/models/tx/KernelType.h>
#include <boost/optional.hpp>

class Kernel :
    public Traits::ICommitted,
    public Traits::IHashable,
    public Traits::ISerializable
{
public:
    Kernel() = default;
    Kernel(
        const uint8_t features,
        const uint64_t fee,
        const uint64_t lockHeight,
        const uint64_t amount,
        boost::optional<Bech32Address>&& address,
        std::vector<uint8_t>&& extraData,
        Commitment&& excess,
        Signature&& signature
    ) : m_features(features),
        m_fee(fee),
        m_lockHeight(lockHeight),
        m_excess(std::move(excess)),
        m_signature(std::move(signature)),
        m_amount(amount),
        m_address(std::move(address)),
        m_extraData(std::move(extraData))
    {
        m_hash = Hashed(*this);
    }

    //
    // Factories
    //
    static Kernel CreatePlain(const BlindingFactor& blind, const uint64_t fee);
    static Kernel CreatePegIn(const BlindingFactor& blind, const uint64_t amount);
    static Kernel CreatePegOut(
        const BlindingFactor& blind,
        const uint64_t amount,
        const uint64_t fee,
        const Bech32Address& address
    );
    static Kernel CreateHeightLocked(
        const BlindingFactor& blind,
        const uint64_t fee,
        const uint64_t lock_height
    );

    //
    // Operators
    //
    bool operator<(const Kernel& rhs) const { return GetHash() < rhs.GetHash(); }
    bool operator==(const Kernel& rhs) const { return GetHash() == rhs.GetHash(); }
    bool operator!=(const Kernel& rhs) const { return GetHash() != rhs.GetHash(); }

    //
    // Getters
    //
    const uint8_t GetFeatures() const noexcept { return m_features; }
    uint64_t GetFee() const noexcept { return m_fee; }
    uint64_t GetLockHeight() const noexcept { return m_lockHeight; }
    const Commitment& GetExcess() const noexcept { return m_excess; }
    const Signature& GetSignature() const noexcept { return m_signature; }
    const std::vector<uint8_t>& GetExtraData() const noexcept { return m_extraData; }

    mw::Hash GetSignatureMessage() const;

    bool IsPegIn() const noexcept { return GetFeatures() == KernelType::PEGIN_KERNEL; }
    bool IsPegOut() const noexcept { return GetFeatures() == KernelType::PEGOUT_KERNEL; }

    uint64_t GetPeggedIn() const noexcept { return IsPegIn() ? m_amount : 0; }
    uint64_t GetPeggedOut() const noexcept { return IsPegOut() ? m_amount : 0; }

    uint64_t GetAmount() const noexcept { return m_amount; }
    const boost::optional<Bech32Address>& GetAddress() const noexcept { return m_address; }

    //
    // Serialization/Deserialization
    //
    Serializer& Serialize(Serializer& serializer) const noexcept final;
    static Kernel Deserialize(Deserializer& deserializer);

    //
    // Traits
    //
    mw::Hash GetHash() const noexcept final { return m_hash; }

    const Commitment& GetCommitment() const noexcept final { return m_excess; }

private:
    // Options for a kernel's structure or use
    uint8_t m_features;

    // Fee originally included in the transaction this proof is for.
    uint64_t m_fee;

    // This kernel is not valid earlier than m_lockHeight blocks
    // The max m_lockHeight of all *inputs* to this transaction
    uint64_t m_lockHeight;

    // Remainder of the sum of all transaction commitments. 
    // If the transaction is well formed, amounts components should sum to zero and the excess is hence a valid public key.
    Commitment m_excess;

    // The signature proving the excess is a valid public key, which signs the transaction fee.
    Signature m_signature;

    mw::Hash m_hash;
    uint64_t m_amount;
    boost::optional<Bech32Address> m_address;
    std::vector<uint8_t> m_extraData;
};

// Sorts so that all pegin kernels are first, then are ordered by hash.
static struct
{
    bool operator()(const Kernel& a, const Kernel& b) const
    {
        return (a.IsPegIn() && !b.IsPegIn())
            || (a.IsPegIn() == b.IsPegIn() && a.GetHash() < b.GetHash());
    }
} KernelSort;