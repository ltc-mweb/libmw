#include <mw/models/tx/Output.h>
#include <mw/models/wallet/StealthAddress.h>
#include <mw/crypto/Random.h>
#include <mw/crypto/Bulletproofs.h>

Output Output::Create(
    BlindingFactor& blind_out,
    const EOutputFeatures features,
    const SecretKey& sender_privkey,
    const StealthAddress& receiver_addr,
    const uint64_t value)
{
    OwnerData owner_data = OwnerData::Create(
        blind_out,
        features,
        sender_privkey,
        receiver_addr,
        value
    );

    // TODO: Determine how to use bulletproof rewind messages.
    // Probably best to store sender_key so sender can identify all outputs they've sent?
    RangeProof::CPtr pRangeProof = Bulletproofs::Generate(
        value,
        SecretKey(blind_out.vec()),
        Random::CSPRNG<32>(),
        Random::CSPRNG<32>(),
        ProofMessage{},
        owner_data.Serialized()
    );

    return Output{
        Crypto::CommitBlinded(value, blind_out),
        std::move(owner_data),
        pRangeProof
    };
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