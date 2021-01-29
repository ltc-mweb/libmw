#include <catch.hpp>

#include <mw/crypto/Crypto.h>
#include <mw/crypto/Random.h>
#include <mw/models/tx/Output.h>
#include <mw/models/wallet/StealthAddress.h>

TEST_CASE("Plain Tx Output")
{
    EOutputFeatures features = EOutputFeatures::DEFAULT_OUTPUT;
    uint64_t amount = 1'234'567;
    BlindingFactor blind;
    OwnerData ownerData = OwnerData::Create(
        blind,
        features,
        Random::CSPRNG<32>(),
        StealthAddress::Random(),
        amount
    );
    RangeProof::CPtr rangeProof = std::make_shared<const RangeProof>(
        std::vector<uint8_t>(Random::CSPRNG<600>().vec())
    );

    Commitment commit = Crypto::CommitBlinded(amount, blind);
    Output output(
        Commitment(commit),
        OwnerData(ownerData),
        rangeProof
    );

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = output.Serialized();

        Deserializer deserializer(serialized);
        REQUIRE(Commitment::Deserialize(deserializer) == commit);
        REQUIRE(OwnerData::Deserialize(deserializer) == ownerData);
        REQUIRE(RangeProof::Deserialize(deserializer) == *rangeProof);

        Deserializer deserializer2(serialized);
        REQUIRE(output == Output::Deserialize(deserializer2));

        REQUIRE(output.GetHash() == Hashed(serialized));
    }

    //
    // Getters
    //
    {
        REQUIRE_FALSE(output.IsPeggedIn());
        REQUIRE(output.GetCommitment() == commit);
        REQUIRE(output.GetOwnerData() == ownerData);
        REQUIRE(output.GetRangeProof() == rangeProof);

        Serializer serializer;
        serializer.Append<uint8_t>((uint8_t)ownerData.GetFeatures()).Append(commit);
        REQUIRE(output.ToIdentifier().GetHash() == Hashed(serializer.vec()));
    }
}