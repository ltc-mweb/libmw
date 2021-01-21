#include <test_framework/TxBuilder.h>

#include <mw/crypto/Keys.h>

TEST_NAMESPACE

TxBuilder::TxBuilder()
    : m_amount{ 0 }, m_kernelOffset{}, m_ownerOffset{}, m_inputs{}, m_outputs{}, m_kernels{}
{

}

TxBuilder& TxBuilder::AddInput(const uint64_t amount, const EOutputFeatures features, const BlindingFactor& blind)
{
    return AddInput(amount, Random::CSPRNG<32>(), features, blind);
}

TxBuilder& TxBuilder::AddInput(
    const uint64_t amount,
    const SecretKey& privkey,
    const EOutputFeatures features,
    const BlindingFactor& blind)
{
    m_kernelOffset.Sub(blind);

    // TODO: Do we still need to multiply by hash of pubkey?
    m_ownerOffset.Sub(privkey);

    PublicKey pubkey = Crypto::CalculatePublicKey(privkey.GetBigInt());
    Signature sig = Schnorr::Sign(privkey.data(), InputMessage());
    m_inputs.push_back(Input{ Crypto::CommitBlinded(amount, blind), std::move(pubkey), std::move(sig) });
    m_amount += (int64_t)amount;
    return *this;
}

TxBuilder& TxBuilder::AddOutput(const uint64_t amount, const EOutputFeatures features, const BlindingFactor& blind)
{
    return AddOutput(amount, Random::CSPRNG<32>(), StealthAddress::Random(), features, blind);
}

TxBuilder& TxBuilder::AddOutput(
    const uint64_t amount,
    const SecretKey& sender_privkey,
    const StealthAddress& receiver_addr,
    const EOutputFeatures features,
    const BlindingFactor& blind)
{
    m_kernelOffset.Add(blind);
    m_ownerOffset.Add(sender_privkey);

    OwnerData owner_data = TxOutput::CreateOwnerData(features, sender_privkey, receiver_addr);
    RangeProof::CPtr pRangeProof = Bulletproofs::Generate(
        amount,
        SecretKey(blind.vec()),
        SecretKey(),
        SecretKey(),
        ProofMessage(BigInt<20>()),
        owner_data.Serialized()
    );

    Output output{ Crypto::CommitBlinded(amount, blind), std::move(owner_data), pRangeProof };
    m_outputs.push_back(std::move(output));
    m_amount -= (int64_t)amount;
    return *this;
}

TxBuilder& TxBuilder::AddPlainKernel(const uint64_t fee, const bool add_owner_sig)
{
    SecretKey kernel_excess = Random::CSPRNG<32>();
    m_kernelOffset.Sub(kernel_excess);

    Commitment excess_commitment = Crypto::CommitBlinded(0, kernel_excess);
    std::vector<uint8_t> kernel_message = Serializer()
        .Append<uint8_t>(KernelType::PLAIN_KERNEL)
        .Append<uint64_t>(fee)
        .vec();

    Signature signature = Schnorr::Sign(kernel_excess.data(), Hashed(kernel_message));
    Kernel kernel = Kernel::CreatePlain(fee, std::move(excess_commitment), std::move(signature));

    if (add_owner_sig) {
        SecretKey offset = Random::CSPRNG<32>();
        m_ownerOffset.Sub(offset);

        mw::Hash msg_hash = kernel.GetHash();
        Signature sig = Schnorr::Sign(offset.data(), msg_hash);
        m_ownerSigs.push_back(SignedMessage{ msg_hash,  Keys::From(offset).PubKey(), sig });
    }

    m_kernels.push_back(std::move(kernel));
    m_amount -= (int64_t)fee;
    return *this;
}

TxBuilder& TxBuilder::AddPeginKernel(const uint64_t amount, const bool add_owner_sig)
{
    SecretKey kernel_excess = Random::CSPRNG<32>();
    m_kernelOffset.Sub(kernel_excess);

    Commitment excess_commitment = Crypto::CommitBlinded(0, kernel_excess);
    std::vector<uint8_t> kernel_message = Serializer()
        .Append<uint8_t>(KernelType::PEGIN_KERNEL)
        .Append<uint64_t>(amount)
        .vec();

    Signature signature = Schnorr::Sign(kernel_excess.data(), Hashed(kernel_message));
    Kernel kernel = Kernel::CreatePegIn(amount, std::move(excess_commitment), std::move(signature));

    if (add_owner_sig) {
        SecretKey offset = Random::CSPRNG<32>();
        m_ownerOffset.Sub(offset);

        mw::Hash msg_hash = kernel.GetHash();
        Signature sig = Schnorr::Sign(offset.data(), msg_hash);
        m_ownerSigs.push_back(SignedMessage{ msg_hash,  Keys::From(offset).PubKey(), sig });
    }

    m_kernels.push_back(std::move(kernel));
    m_amount += amount;
    return *this;
}

TxBuilder& TxBuilder::AddPegoutKernel(const uint64_t amount, const uint64_t fee, const bool add_owner_sig)
{
    SecretKey kernel_excess = Random::CSPRNG<32>();
    m_kernelOffset.Sub(kernel_excess);
    Bech32Address ltc_address("hrp", Random::CSPRNG<32>().vec());

    Commitment excess_commitment = Crypto::CommitBlinded(0, kernel_excess);
    std::vector<uint8_t> kernel_message = Serializer()
        .Append<uint8_t>(KernelType::PEGOUT_KERNEL)
        .Append<uint64_t>(fee)
        .Append<uint64_t>(amount)
        .Append(ltc_address)
        .vec();

    Signature signature = Schnorr::Sign(kernel_excess.data(), Hashed(kernel_message));
    Kernel kernel = Kernel::CreatePegOut(
        amount,
        fee,
        std::move(ltc_address),
        std::move(excess_commitment),
        std::move(signature)
    );

    if (add_owner_sig) {
        SecretKey offset = Random::CSPRNG<32>();
        m_ownerOffset.Sub(offset);

        mw::Hash msg_hash = kernel.GetHash();
        Signature sig = Schnorr::Sign(offset.data(), msg_hash);
        m_ownerSigs.push_back(SignedMessage{ msg_hash,  Keys::From(offset).PubKey(), sig });
    }

    m_kernels.push_back(std::move(kernel));
    m_amount -= amount + fee;
    return *this;
}

mw::Transaction::CPtr TxBuilder::Build()
{
    assert(m_amount == 0);

    std::sort(m_inputs.begin(), m_inputs.end(), SortByCommitment);
    std::sort(m_outputs.begin(), m_outputs.end(), SortByCommitment);
    std::sort(m_kernels.begin(), m_kernels.end(), SortByHash);
    std::sort(m_ownerSigs.begin(), m_ownerSigs.end(), SortByHash);

    return std::make_shared<mw::Transaction>(
        m_kernelOffset.Total(),
        m_ownerOffset.Total(),
        TxBody{ m_inputs, m_outputs, m_kernels, m_ownerSigs }
    );
}

END_NAMESPACE