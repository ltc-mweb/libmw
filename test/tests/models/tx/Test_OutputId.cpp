#include <catch.hpp>

#include <mw/crypto/Crypto.h>
#include <mw/crypto/Random.h>
#include <mw/models/tx/OutputId.h>

TEST_CASE("Tx Output Identifier")
{
    EOutputFeatures features = EOutputFeatures::DEFAULT_OUTPUT;
    Commitment commit = Random::CSPRNG<33>().GetBigInt();
    OutputId outputId(features, commit);

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = outputId.Serialized();
        REQUIRE(serialized.size() == 34);

        Deserializer deserializer(serialized);
        REQUIRE(deserializer.Read<uint8_t>() == features);
        REQUIRE(Commitment::Deserialize(deserializer) == commit);

        Deserializer deserializer2(serialized);
        REQUIRE(outputId == OutputId::Deserialize(deserializer2));

        REQUIRE(outputId.GetHash() == Hashed(serialized));
    }

    //
    // Getters
    //
    {
        REQUIRE_FALSE(outputId.IsPeggedIn());
        REQUIRE(outputId.GetFeatures() == features);
        REQUIRE(outputId.GetCommitment() == commit);
    }
}