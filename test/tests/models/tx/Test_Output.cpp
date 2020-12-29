#include <catch.hpp>

#include <mw/crypto/Crypto.h>
#include <mw/crypto/Random.h>
#include <mw/models/tx/Output.h>

TEST_CASE("Plain Tx Output")
{
    EOutputFeatures features = EOutputFeatures::DEFAULT_OUTPUT;
    Commitment commit = Random::CSPRNG<33>().GetBigInt();
    OwnerData ownerData(
        features,
        PublicKey(Random::CSPRNG<33>().vec()),
        PublicKey(Random::CSPRNG<33>().vec()),
        PublicKey(Random::CSPRNG<33>().vec()),
        std::vector<uint8_t>{},
        Signature(Random::CSPRNG<64>().vec())
    );
    RangeProof::CPtr rangeProof = std::make_shared<const RangeProof>(
        std::vector<uint8_t>(Random::CSPRNG<600>().vec())
    );

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

TEST_CASE("Peg-In Tx Output")
{
    EOutputFeatures features = EOutputFeatures::PEGGED_IN;
    Commitment commit = Random::CSPRNG<33>().GetBigInt();
    OwnerData ownerData(
        features,
        PublicKey(Random::CSPRNG<33>().vec()),
        PublicKey(Random::CSPRNG<33>().vec()),
        PublicKey(Random::CSPRNG<33>().vec()),
        std::vector<uint8_t>{},
        Signature(Random::CSPRNG<64>().vec())
    );
    RangeProof::CPtr rangeProof = std::make_shared<const RangeProof>(
        std::vector<uint8_t>(Random::CSPRNG<600>().vec())
    );

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
        REQUIRE(output.IsPeggedIn());
        REQUIRE(output.GetFeatures() == features);
        REQUIRE(output.GetCommitment() == commit);
        REQUIRE(output.GetOwnerData() == ownerData);
        REQUIRE(output.GetRangeProof() == rangeProof);

        Serializer serializer;
        serializer.Append<uint8_t>((uint8_t)features).Append(commit);
        REQUIRE(output.ToIdentifier().GetHash() == Hashed(serializer.vec()));
    }
}