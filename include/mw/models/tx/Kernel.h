#pragma once

#include <mw/crypto/Crypto.h>
#include <mw/crypto/Hasher.h>
#include <mw/traits/Committed.h>
#include <mw/traits/Hashable.h>
#include <mw/traits/Serializable.h>
#include <mw/traits/Printable.h>
#include <mw/traits/Jsonable.h>
#include <mw/models/crypto/Bech32Address.h>
#include <mw/models/crypto/Signature.h>
#include <mw/models/tx/KernelType.h>
#include <boost/optional.hpp>

class Kernel :
    public Traits::ICommitted,
    public Traits::IHashable,
    public Traits::ISerializable,
    public Traits::IJsonable
{
public:
    using CPtr = std::shared_ptr<const Kernel>;

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
        m_hash(boost::none),
        m_amount(amount),
        m_address(std::move(address)),
        m_extraData(std::move(extraData))
    { }

    static Kernel CreatePlain(const uint64_t fee, Commitment&& excess, Signature&& signature)
    {
        return Kernel(
            KernelType::PLAIN_KERNEL,
            fee,
            0,
            0,
            boost::none,
            std::vector<uint8_t>(),
            std::move(excess),
            std::move(signature)
        );
    }

    static Kernel CreatePegIn(const uint64_t amount, Commitment&& excess, Signature&& signature)
    {
        return Kernel(
            KernelType::PEGIN_KERNEL,
            0,
            0, // TODO: Can peg-in kernels have lock-heights?
            amount,
            boost::none,
            std::vector<uint8_t>(),
            std::move(excess),
            std::move(signature)
        );
    }

    static Kernel CreatePegOut(const uint64_t amount, const uint64_t fee, Bech32Address&& address, Commitment&& excess, Signature&& signature)
    {
        return Kernel(
            KernelType::PEGOUT_KERNEL,
            fee,
            0, // TODO: Can peg-out kernels have lock-heights?
            amount,
            boost::make_optional<Bech32Address>(std::move(address)),
            std::vector<uint8_t>(),
            std::move(excess),
            std::move(signature)
        );
    }

    static Kernel CreateHeightLocked(const uint64_t fee, const uint64_t lockHeight, Commitment&& excess, Signature&& signature)
    {
        return Kernel(
            KernelType::HEIGHT_LOCKED,
            fee,
            lockHeight,
            0,
            boost::none,
            std::vector<uint8_t>(),
            std::move(excess),
            std::move(signature)
        );
    }

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

    mw::Hash GetSignatureMessage() const
    {
        Serializer serializer;
        serializer.Append<uint8_t>(m_features);

        switch (m_features)
        {
            case KernelType::PLAIN_KERNEL:
            {
                serializer.Append<uint64_t>(m_fee);
                break;
            }
            case KernelType::PEGIN_KERNEL:
            {
                serializer.Append<uint64_t>(m_amount);
                break;
            }
            case KernelType::PEGOUT_KERNEL:
            {
                serializer.Append<uint64_t>(m_fee);
                serializer.Append<uint64_t>(m_amount);
                serializer.Append(m_address.value());
                break;
            }
            case KernelType::HEIGHT_LOCKED:
            {
                serializer.Append<uint64_t>(m_fee);
                serializer.Append<uint64_t>(m_lockHeight);
                break;
            }
            default:
            {
                serializer.Append<uint64_t>(m_fee);
                serializer.Append(m_extraData);
            }
        }

        return Hashed(serializer.vec());
    }

    bool IsPegIn() const noexcept { return GetFeatures() == KernelType::PEGIN_KERNEL; }
    bool IsPegOut() const noexcept { return GetFeatures() == KernelType::PEGOUT_KERNEL; }

    uint64_t GetPeggedIn() const noexcept { return IsPegIn() ? m_amount : 0; }
    uint64_t GetPeggedOut() const noexcept { return IsPegOut() ? m_amount : 0; }

    uint64_t GetAmount() const noexcept { return m_amount; }
    const boost::optional<Bech32Address>& GetAddress() const noexcept { return m_address; }

    //
    // Serialization/Deserialization
    //
    Serializer& Serialize(Serializer& serializer) const noexcept final
    {
        serializer.Append<uint8_t>(m_features);

        switch ((KernelType::EKernelType)m_features)
        {
            case KernelType::PLAIN_KERNEL:
            {
                serializer.Append<uint64_t>(m_fee);
                break;
            }
            case KernelType::PEGIN_KERNEL:
            {
                serializer.Append<uint64_t>(m_amount);
                break;
            }
            case KernelType::PEGOUT_KERNEL:
            {
                serializer.Append<uint64_t>(m_fee);
                serializer.Append<uint64_t>(m_amount);
                serializer.Append(m_address.value());
                break;
            }
            case KernelType::HEIGHT_LOCKED:
            {
                serializer.Append<uint64_t>(m_fee);
                serializer.Append<uint64_t>(m_lockHeight);
                break;
            }
            default:
            {
                serializer.Append<uint64_t>(m_fee);
                serializer.Append<uint8_t>((uint8_t)m_extraData.size());
                serializer.Append(m_extraData);
            }
        }

        return serializer
            .Append(m_excess)
            .Append(m_signature);
    }

    static Kernel Deserialize(Deserializer& deserializer)
    {
        uint8_t type = deserializer.Read<uint8_t>();

        uint64_t fee = 0;
        uint64_t lockHeight = 0;
        uint64_t amount = 0;
        boost::optional<Bech32Address> address = boost::none;
        std::vector<uint8_t> extraData{};

        switch (type)
        {
            case KernelType::PLAIN_KERNEL:
            {
                fee = deserializer.Read<uint64_t>();
                break;
            }
            case KernelType::PEGIN_KERNEL:
            {
                amount = deserializer.Read<uint64_t>();
                break;
            }
            case KernelType::PEGOUT_KERNEL:
            {
                fee = deserializer.Read<uint64_t>();
                amount = deserializer.Read<uint64_t>();
                address = boost::make_optional(Bech32Address::Deserialize(deserializer));
                break;
            }
            case KernelType::HEIGHT_LOCKED:
            {
                fee = deserializer.Read<uint64_t>();
                lockHeight = deserializer.Read<uint64_t>();
                break;
            }
            default:
            {
                fee = deserializer.Read<uint64_t>();

                uint8_t messageSize = deserializer.Read<uint8_t>();
                if (messageSize > 0)
                {
                    extraData = deserializer.ReadVector(messageSize);
                }
            }
        }

        Commitment excess = Commitment::Deserialize(deserializer);
        Signature signature = Signature::Deserialize(deserializer);

        return Kernel{
            type,
            fee,
            lockHeight,
            amount,
            std::move(address),
            std::move(extraData),
            std::move(excess),
            std::move(signature)
        };
    }

    json ToJSON() const noexcept final
    {
        json json({
            { "type", KernelType::ToString(m_features) },
            { "excess", m_excess },
            { "signature", m_signature }
        });

        switch (m_features)
        {
            case KernelType::PLAIN_KERNEL:
            {
                json["fee"] = m_fee;
                break;
            }
            case KernelType::PEGIN_KERNEL:
            {
                json["amount"] = m_amount;
                break;
            }
            case KernelType::PEGOUT_KERNEL:
            {
                json["fee"] = m_fee;
                json["amount"] = m_amount;
                json["address"] = m_address.value();
                break;
            }
            case KernelType::HEIGHT_LOCKED:
            {
                json["fee"] = m_fee;
                json["lock_height"] = m_lockHeight;
                break;
            }
        }

        return json;
    }

    static Kernel FromJSON(const Json& json)
    {
        uint8_t type = KernelType::FromString(json.GetRequired<std::string>("type"));
        uint64_t fee = json.GetOr<uint64_t>("fee", 0);
        uint64_t lockHeight = json.GetOr<uint64_t>("lock_height", 0);
        uint64_t amount = json.GetOr<uint64_t>("amount", 0);
        boost::optional<Bech32Address> address = boost::none;
        if (json.Exists("address"))
        {
            address = boost::make_optional(json.GetRequired<Bech32Address>("address"));
        }

        Commitment excess = json.GetRequired<Commitment>("excess");
        Signature signature = json.GetRequired<Signature>("signature");

        return Kernel(
            type,
            fee,
            lockHeight,
            amount,
            std::move(address),
            std::vector<unsigned char>(),
            std::move(excess),
            std::move(signature)
        );
    }

    //
    // Traits
    //
    mw::Hash GetHash() const noexcept final
    {
        if (!m_hash.has_value())
        {
            m_hash = boost::make_optional(Hashed(*this));
        }

        return m_hash.value();
    }

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

    mutable boost::optional<mw::Hash> m_hash;
    uint64_t m_amount;
    boost::optional<Bech32Address> m_address;
    std::vector<uint8_t> m_extraData;
};