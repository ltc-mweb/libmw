#include <catch.hpp>

#include <mw/crypto/Crypto.h>
#include <mw/crypto/Random.h>
#include <mw/models/tx/Output.h>

TEST_CASE("Plain Tx Output")
{
    EOutputFeatures features = EOutputFeatures::DEFAULT_OUTPUT;
    Commitment commit = Random::CSPRNG<33>().GetBigInt();
    OwnerData ownerData{}; // TODO: Populate this
    RangeProof::CPtr rangeProof = std::make_shared<const RangeProof>(
        std::vector<uint8_t>(Random::CSPRNG<600>().vec())
    );

    Output output(
        features,
        Commitment(commit),
        OwnerData(ownerData),
        rangeProof
    );

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = output.Serialized();
        REQUIRE(serialized.size() == 806);

        Deserializer deserializer(serialized);
        REQUIRE(deserializer.Read<uint8_t>() == features);
        REQUIRE(Commitment::Deserialize(deserializer) == commit);
        REQUIRE(OwnerData::Deserialize(deserializer) == ownerData);
        REQUIRE(RangeProof::Deserialize(deserializer) == *rangeProof);

        Deserializer deserializer2(serialized);
        REQUIRE(output == Output::Deserialize(deserializer2));

        REQUIRE(output.GetHash() == Hashed(serialized));
    }

    //
    // JSON
    //
    {
        Json json(output.ToJSON());
        REQUIRE(json.GetKeys() == std::vector<std::string>({ "commit", "features", "owner_data", "proof" }));
        REQUIRE(json.GetRequired<std::string>("features") == "Plain");
        REQUIRE(json.GetRequired<Commitment>("commit") == commit);
        REQUIRE(json.GetRequired<OwnerData>("owner_data") == ownerData);
        REQUIRE(json.GetRequired<RangeProof>("proof") == *rangeProof);

        REQUIRE(output == Output::FromJSON(json));
    }

    //
    // Getters
    //
    {
        REQUIRE_FALSE(output.IsPeggedIn());
        REQUIRE(output.GetFeatures() == features);
        REQUIRE(output.GetCommitment() == commit);
        REQUIRE(output.GetOwnerData() == ownerData);
        REQUIRE(output.GetRangeProof() == rangeProof);

        Serializer serializer;
        serializer.Append<uint8_t>((uint8_t)features).Append(commit);
        REQUIRE(output.ToIdentifier().GetHash() == Hashed(serializer.vec()));
    }
}

TEST_CASE("Peg-In Tx Output")
{
    EOutputFeatures features = EOutputFeatures::PEGGED_IN;
    Commitment commit = Random::CSPRNG<33>().GetBigInt();
    OwnerData ownerData{}; // TODO: Populate this
    RangeProof::CPtr rangeProof = std::make_shared<const RangeProof>(
        std::vector<uint8_t>(Random::CSPRNG<600>().vec())
    );

    Output output(
        features,
        Commitment(commit),
        OwnerData(ownerData),
        rangeProof
    );

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = output.Serialized();
        REQUIRE(serialized.size() == 806);

        Deserializer deserializer(serialized);
        REQUIRE(deserializer.Read<uint8_t>() == features);
        REQUIRE(Commitment::Deserialize(deserializer) == commit);
        REQUIRE(OwnerData::Deserialize(deserializer) == ownerData);
        REQUIRE(RangeProof::Deserialize(deserializer) == *rangeProof);

        Deserializer deserializer2(serialized);
        REQUIRE(output == Output::Deserialize(deserializer2));

        REQUIRE(output.GetHash() == Hashed(serialized));
    }

    //
    // JSON
    //
    {
        Json json(output.ToJSON());
        REQUIRE(json.GetKeys() == std::vector<std::string>({ "commit", "features", "owner_data", "proof" }));
        REQUIRE(json.GetRequired<std::string>("features") == "PeggedIn");
        REQUIRE(json.GetRequired<Commitment>("commit") == commit);
        REQUIRE(json.GetRequired<OwnerData>("owner_data") == ownerData);
        REQUIRE(json.GetRequired<RangeProof>("proof") == *rangeProof);

        REQUIRE(output == Output::FromJSON(json));
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