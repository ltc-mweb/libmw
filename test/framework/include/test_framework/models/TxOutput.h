#pragma once

#include <mw/common/Macros.h>
#include <mw/models/tx/Output.h>
#include <mw/models/wallet/StealthAddress.h>
#include <mw/crypto/Bulletproofs.h>
#include <mw/crypto/Random.h>

TEST_NAMESPACE

class TxOutput
{
public:
    TxOutput(const BlindingFactor& blindingFactor, const uint64_t amount, const Output& output)
        : m_blindingFactor(blindingFactor), m_amount(amount), m_output(output) { }

    static TxOutput Create(
        const EOutputFeatures features,
        const BlindingFactor& blindingFactor,
        const SecretKey& sender_privkey,
        const StealthAddress& receiver_addr,
        const uint64_t amount)
    {
        Commitment commitment = Crypto::CommitBlinded(
            amount,
            blindingFactor
        );

        OwnerData owner_data = CreateOwnerData(features, sender_privkey, receiver_addr);

        RangeProof::CPtr pRangeProof = Bulletproofs::Generate(
            amount,
            SecretKey(blindingFactor.vec()),
            SecretKey(),
            SecretKey(),
            ProofMessage(BigInt<20>()),
            owner_data.Serialized()
        );

        return TxOutput(
            blindingFactor,
            amount,
            Output{ std::move(commitment), std::move(owner_data), pRangeProof }
        );
    }

    // TODO: Use OutputFactory instead
    static OwnerData CreateOwnerData(
        const EOutputFeatures features,
        const SecretKey& sender_privkey,
        const StealthAddress& receiver_addr)
    {
        PublicKey sender_pubkey = Keys::From(sender_privkey).PubKey();
        SecretKey r = Random::CSPRNG<32>();
        PublicKey R = Keys::From(r).PubKey();
        PublicKey rA = Keys::From(receiver_addr.A()).Mul(r).PubKey();
        PublicKey receiver_pubkey = Keys::From(Hashed(rA)).Add(receiver_addr.B()).PubKey();
        std::vector<uint8_t> encrypted_data{}; // TODO: Encrypt blinding factor & amount

        auto serialized_msg = Serializer()
            .Append<uint8_t>(features)
            .Append(receiver_pubkey)
            .Append(R)
            .Append<uint8_t>((uint8_t)encrypted_data.size())
            .Append(encrypted_data)
            .vec();
        Signature signature = Schnorr::Sign(sender_privkey.data(), Hashed(serialized_msg));

        return OwnerData(
            features,
            std::move(sender_pubkey),
            std::move(receiver_pubkey),
            std::move(R),
            std::move(encrypted_data),
            std::move(signature)
        );
    }

    const BlindingFactor& GetBlindingFactor() const noexcept { return m_blindingFactor; }
    uint64_t GetAmount() const noexcept { return m_amount; }
    const Output& GetOutput() const noexcept { return m_output; }

private:
    BlindingFactor m_blindingFactor;
    uint64_t m_amount;
    Output m_output;
};

END_NAMESPACE