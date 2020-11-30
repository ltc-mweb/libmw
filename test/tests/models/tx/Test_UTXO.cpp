#include <catch.hpp>

#include <mw/crypto/Crypto.h>
#include <mw/crypto/Random.h>
#include <mw/models/tx/UTXO.h>

TEST_CASE("Tx UTXO")
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

    uint64_t blockHeight = 20;
    mmr::LeafIndex leafIndex = mmr::LeafIndex::At(5);
    UTXO utxo{
        blockHeight,
        mmr::LeafIndex(leafIndex),
        Output(output)
    };

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = utxo.Serialized();
        REQUIRE(serialized.size() == 822);

        Deserializer deserializer(serialized);
        REQUIRE(deserializer.Read<uint64_t>() == blockHeight);
        REQUIRE(mmr::LeafIndex::At(deserializer.Read<uint64_t>()) == leafIndex);
        REQUIRE(Output::Deserialize(deserializer) == output);
    }

    //
    // Getters
    //
    {
        REQUIRE(utxo.GetBlockHeight() == blockHeight);
        REQUIRE(utxo.GetLeafIndex() == leafIndex);
        REQUIRE(utxo.GetOutput() == output);
        REQUIRE(utxo.GetCommitment() == commit);
        REQUIRE(utxo.GetOwnerData() == ownerData);
        REQUIRE(utxo.GetRangeProof() == rangeProof);
    }
}