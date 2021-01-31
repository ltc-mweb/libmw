#include <mw/models/tx/Output.h>
#include <mw/models/wallet/StealthAddress.h>
#include <mw/crypto/Random.h>
#include <mw/crypto/Bulletproofs.h>

Output Output::Create(
    const EOutputFeatures features,
    const BlindingFactor& blinding_factor,
    const SecretKey& sender_privkey,
    const StealthAddress& receiver_addr,
    const uint64_t amount)
{
    Commitment commitment = Crypto::CommitBlinded(
        amount,
        blinding_factor
    );

    OwnerData owner_data = OwnerData::Create(
        features,
        sender_privkey,
        receiver_addr,
        blinding_factor,
        amount
    );

    // TODO: Determine how to use bulletproof rewind messages.
    // Probably best to store sender_key so sender can identify all outputs they've sent?
    RangeProof::CPtr pRangeProof = Bulletproofs::Generate(
        amount,
        SecretKey(blinding_factor.vec()),
        Random::CSPRNG<32>(),
        Random::CSPRNG<32>(),
        ProofMessage{},
        owner_data.Serialized()
    );

    return Output{ std::move(commitment), std::move(owner_data), pRangeProof };
}

Serializer& Output::Serialize(Serializer& serializer) const noexcept
{
    return serializer
        .Append(m_commitment)
        .Append(m_ownerData)
        .Append(m_pProof);
}

Output Output::Deserialize(Deserializer& deserializer)
{
    Commitment commitment = Commitment::Deserialize(deserializer);
    OwnerData owner_data = OwnerData::Deserialize(deserializer);
    RangeProof::CPtr pProof = std::make_shared<const RangeProof>(RangeProof::Deserialize(deserializer));
    return Output(std::move(commitment), std::move(owner_data), pProof);
}