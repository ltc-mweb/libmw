#include <catch.hpp>

#include <mw/crypto/Crypto.h>
#include <mw/crypto/Random.h>
#include <mw/models/tx/Input.h>

TEST_CASE("Plain Tx Input")
{
    Commitment commit(Random::CSPRNG<33>().GetBigInt());
    Input input(EOutputFeatures::DEFAULT_OUTPUT, Commitment(commit));

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = input.Serialized();
        REQUIRE(serialized.size() == 34);

        Deserializer deserializer(serialized);
        REQUIRE(deserializer.Read<uint8_t>() == EOutputFeatures::DEFAULT_OUTPUT);
        REQUIRE(Commitment::Deserialize(deserializer) == commit);

        Deserializer deserializer2(serialized);
        REQUIRE(input == Input::Deserialize(deserializer2));

        REQUIRE(input.GetHash() == Hashed(serialized));
    }

    //
    // JSON
    //
    {
        Json json(input.ToJSON());
        REQUIRE(json.GetKeys() == std::vector<std::string>({ "commit", "features" }));
        REQUIRE(json.GetRequired<std::string>("features") == "Plain");
        REQUIRE(json.GetRequired<Commitment>("commit") == commit);

        REQUIRE(input == Input::FromJSON(json));
    }

    //
    // Getters
    //
    {
        REQUIRE_FALSE(input.IsPeggedIn());
        REQUIRE(input.GetFeatures() == EOutputFeatures::DEFAULT_OUTPUT);
        REQUIRE(input.GetCommitment() == commit);
    }
}

TEST_CASE("Peg-In Tx Input")
{
    Commitment commit(Random::CSPRNG<33>().GetBigInt());
    Input input(EOutputFeatures::PEGGED_IN, Commitment(commit));

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = input.Serialized();
        REQUIRE(serialized.size() == 34);

        Deserializer deserializer(serialized);
        REQUIRE(deserializer.Read<uint8_t>() == EOutputFeatures::PEGGED_IN);
        REQUIRE(Commitment::Deserialize(deserializer) == commit);

        Deserializer deserializer2(serialized);
        REQUIRE(input == Input::Deserialize(deserializer2));

        REQUIRE(input.GetHash() == Hashed(serialized));
    }

    //
    // JSON
    //
    {
        Json json(input.ToJSON());
        REQUIRE(json.GetKeys() == std::vector<std::string>({ "commit", "features" }));
        REQUIRE(json.GetRequired<std::string>("features") == "PeggedIn");
        REQUIRE(json.GetRequired<Commitment>("commit") == commit);

        REQUIRE(input == Input::FromJSON(json));
    }

    //
    // Getters
    //
    {
        REQUIRE(input.IsPeggedIn());
        REQUIRE(input.GetFeatures() == EOutputFeatures::PEGGED_IN);
        REQUIRE(input.GetCommitment() == commit);
    }
}