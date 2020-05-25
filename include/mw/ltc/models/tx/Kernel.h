#pragma once

#include <mw/core/models/tx/IKernel.h>
#include <mw/core/models/crypto/Bech32Address.h>
#include <mw/ltc/models/tx/KernelType.h>

// TODO: Create CoinbaseKernel, PegInKernel, PegoutKernel?
class Kernel : public IKernel
{
public:
    using CPtr = std::shared_ptr<const Kernel>;

    Kernel(
        const uint8_t features,
        const uint64_t fee,
        const uint64_t lockHeight,
        const uint64_t amount,
        tl::optional<Bech32Address>&& address,
        std::vector<uint8_t>&& extraData,
        Commitment&& excess,
        Signature&& signature
    ) : IKernel(features, fee, lockHeight, std::move(excess), std::move(signature)),
        m_amount(amount),
        m_address(std::move(address)),
        m_extraData(std::move(extraData))
    { }

    static Kernel::CPtr CreatePlain(const uint64_t fee, Commitment&& excess, Signature&& signature)
    {
        return std::make_shared<Kernel>(
            KernelType::PLAIN_KERNEL,
            fee,
            0,
            0,
            tl::nullopt,
            std::vector<uint8_t>(),
            std::move(excess),
            std::move(signature)
        );
    }

    static Kernel::CPtr CreatePegIn(const uint64_t amount, Commitment&& excess, Signature&& signature)
    {
        return std::make_shared<Kernel>(
            KernelType::PEGIN_KERNEL,
            0,
            0, // TODO: Can peg-in kernels have lock-heights?
            amount,
            tl::nullopt,
            std::vector<uint8_t>(),
            std::move(excess),
            std::move(signature)
        );
    }

    static Kernel::CPtr CreatePegOut(const uint64_t amount, const uint64_t fee, Bech32Address&& address, Commitment&& excess, Signature&& signature)
    {
        return std::make_shared<Kernel>(
            KernelType::PEGOUT_KERNEL,
            fee,
            0, // TODO: Can peg-out kernels have lock-heights?
            amount,
            tl::make_optional<Bech32Address>(std::move(address)),
            std::vector<uint8_t>(),
            std::move(excess),
            std::move(signature)
        );
    }

    static Kernel::CPtr CreateHeightLocked(const uint64_t fee, const uint64_t lockHeight, Commitment&& excess, Signature&& signature)
    {
        return std::make_shared<Kernel>(
            KernelType::HEIGHT_LOCKED,
            fee,
            lockHeight,
            0,
            tl::nullopt,
            std::vector<uint8_t>(),
            std::move(excess),
            std::move(signature)
        );
    }

    //
    // Operators
    //
    bool operator<(const IKernel& rhs) const { return this->IKernel::operator<(rhs); }
    bool operator==(const IKernel& rhs) const { return this->IKernel::operator==(rhs); }
    bool operator!=(const IKernel& rhs) const { return this->IKernel::operator!=(rhs); }

    //
    // Getters
    //
    Hash GetSignatureMessage() const final
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

        return Crypto::Blake2b(serializer.vec());
    }

    bool IsPegIn() const noexcept { return GetFeatures() == KernelType::PEGIN_KERNEL; }
    bool IsPegOut() const noexcept { return GetFeatures() == KernelType::PEGOUT_KERNEL; }

    uint64_t GetPeggedIn() const noexcept { return IsPegIn() ? m_amount : 0; }
    uint64_t GetPeggedOut() const noexcept { return IsPegOut() ? m_amount : 0; }

    uint64_t GetAmount() const noexcept { return m_amount; }
    const tl::optional<Bech32Address>& GetAddress() const noexcept { return m_address; }

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

    static Kernel::CPtr Deserialize(Deserializer& deserializer)
    {
        uint8_t type = deserializer.Read<uint8_t>();

        uint64_t fee = 0;
        uint64_t lockHeight = 0;
        uint64_t amount = 0;
        tl::optional<Bech32Address> address = tl::nullopt;
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
                address = tl::make_optional(Bech32Address::Deserialize(deserializer));
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

        return std::make_shared<Kernel>(
            type,
            fee,
            lockHeight,
            amount,
            std::move(address),
            std::move(extraData),
            std::move(excess),
            std::move(signature)
        );
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

    static Kernel::CPtr FromJSON(const Json& json)
    {
        uint8_t type = KernelType::FromString(json.GetRequired<std::string>("type"));
        uint64_t fee = json.GetOr<uint64_t>("fee", 0);
        uint64_t lockHeight = json.GetOr<uint64_t>("lock_height", 0);
        uint64_t amount = json.GetOr<uint64_t>("amount", 0);
        tl::optional<Bech32Address> address = tl::nullopt;
        if (json.Exists("address"))
        {
            address = tl::make_optional(json.GetRequired<Bech32Address>("address"));
        }

        Commitment excess = json.GetRequired<Commitment>("excess");
        Signature signature = json.GetRequired<Signature>("signature");

        return std::make_shared<Kernel>(
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

private:
    uint64_t m_amount;
    tl::optional<Bech32Address> m_address;
    std::vector<uint8_t> m_extraData;
};