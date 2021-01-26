#include <mw/models/tx/Kernel.h>
#include <mw/crypto/Schnorr.h>

Kernel Kernel::CreatePlain(const BlindingFactor& blind, const uint64_t fee)
{
    Commitment excess_commit = Crypto::CommitBlinded(0, blind);
    mw::Hash sig_message = Hasher()
        .Append<uint8_t>(KernelType::PLAIN_KERNEL)
        .Append<uint64_t>(fee)
        .hash();
    Signature sig = Schnorr::Sign(blind.data(), sig_message);

    return Kernel(
        KernelType::PLAIN_KERNEL,
        fee,
        0,
        0,
        boost::none,
        std::vector<uint8_t>{ },
        std::move(excess_commit),
        std::move(sig)
    );
}

Kernel Kernel::CreatePegIn(const BlindingFactor& blind, const uint64_t amount)
{
    Commitment excess_commit = Crypto::CommitBlinded(0, blind);
    mw::Hash sig_message = Hasher()
        .Append<uint8_t>(KernelType::PEGIN_KERNEL)
        .Append<uint64_t>(amount)
        .hash();
    Signature sig = Schnorr::Sign(blind.data(), sig_message);

    return Kernel(
        KernelType::PEGIN_KERNEL,
        0,
        0,
        amount,
        boost::none,
        std::vector<uint8_t>{ },
        std::move(excess_commit),
        std::move(sig)
    );
}

Kernel Kernel::CreatePegOut(
    const BlindingFactor& blind,
    const uint64_t amount,
    const uint64_t fee,
    const Bech32Address& address)
{
    Commitment excess_commit = Crypto::CommitBlinded(0, blind);
    mw::Hash sig_message = Hasher()
        .Append<uint8_t>(KernelType::PEGOUT_KERNEL)
        .Append<uint64_t>(fee)
        .Append<uint64_t>(amount)
        .Append(address)
        .hash();
    Signature sig = Schnorr::Sign(blind.data(), sig_message);

    return Kernel(
        KernelType::PEGOUT_KERNEL,
        fee,
        0, // TODO: Can peg-out kernels have lock-heights?
        amount,
        boost::make_optional<Bech32Address>(Bech32Address(address)),
        std::vector<uint8_t>{ },
        std::move(excess_commit),
        std::move(sig)
    );
}

Kernel Kernel::CreateHeightLocked(const BlindingFactor& blind, const uint64_t fee, const uint64_t lock_height)
{
    Commitment excess_commit = Crypto::CommitBlinded(0, blind);
    mw::Hash sig_message = Hasher()
        .Append<uint8_t>(KernelType::HEIGHT_LOCKED)
        .Append<uint64_t>(fee)
        .Append<uint64_t>(lock_height)
        .hash();
    Signature sig = Schnorr::Sign(blind.data(), sig_message);

    return Kernel(
        KernelType::HEIGHT_LOCKED,
        fee,
        lock_height,
        0,
        boost::none,
        std::vector<uint8_t>{ },
        std::move(excess_commit),
        std::move(sig)
    );
}

mw::Hash Kernel::GetSignatureMessage() const
{
    Serializer serializer;
    serializer.Append<uint8_t>(m_features);

    switch (m_features) {
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

Serializer& Kernel::Serialize(Serializer& serializer) const noexcept
{
    serializer.Append<uint8_t>(m_features);

    switch ((KernelType::EKernelType)m_features) {
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

Kernel Kernel::Deserialize(Deserializer& deserializer)
{
    uint8_t type = deserializer.Read<uint8_t>();

    uint64_t fee = 0;
    uint64_t lockHeight = 0;
    uint64_t amount = 0;
    boost::optional<Bech32Address> address = boost::none;
    std::vector<uint8_t> extraData{};

    switch (type) {
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
        if (messageSize > 0) {
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