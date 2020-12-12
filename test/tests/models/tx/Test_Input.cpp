#include <catch.hpp>

#include <mw/crypto/Crypto.h>
#include <mw/crypto/Random.h>
#include <mw/models/tx/Input.h>

TEST_CASE("Plain Tx Input")
{
    // TODO: Use Commitment::FromHex instead
    Commitment commit(Random::CSPRNG<33>().GetBigInt());
    Signature signature(Random::CSPRNG<64>().GetBigInt());
    Input input(commit, signature);

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = input.Serialized();
        REQUIRE(serialized.size() == 97);

        Deserializer deserializer(serialized);
        REQUIRE(Commitment::Deserialize(deserializer) == commit);
        REQUIRE(Signature::Deserialize(deserializer) == signature);

        Deserializer deserializer2(serialized);
        REQUIRE(input == Input::Deserialize(deserializer2));

        REQUIRE(input.GetHash() == Hashed(serialized));
    }

    //
    // JSON
    //
    {
        Json json(input.ToJSON());
        REQUIRE(json.GetKeys() == std::vector<std::string>({ "commit", "signature" }));
        REQUIRE(json.GetRequired<Commitment>("commit") == commit);
        REQUIRE(json.GetRequired<Signature>("signature") == signature);

        REQUIRE(input == Input::FromJSON(json));
    }

    //
    // Getters
    //
    {
        REQUIRE(input.GetCommitment() == commit);
        REQUIRE(input.GetSignature() == signature);
    }
}