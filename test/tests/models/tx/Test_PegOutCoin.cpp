#include <catch.hpp>

#include <mw/crypto/Crypto.h>
#include <mw/models/tx/PegOutCoin.h>

TEST_CASE("Tx Peg-Out Coin")
{
    uint64_t amount = 123;
    Bech32Address address = Bech32Address::FromString("bc1qc7slrfxkknqcq2jevvvkdgvrt8080852dfjewde450xdlk4ugp7szw5tk9");
    PegOutCoin pegOutCoin(amount, address);

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = pegOutCoin.Serialized();
        REQUIRE(serialized.size() == 62);

        Deserializer deserializer(serialized);
        REQUIRE(deserializer.Read<uint64_t>() == amount);
        REQUIRE(Bech32Address::Deserialize(deserializer) == address);

        Deserializer deserializer2(serialized);
        REQUIRE(pegOutCoin == PegOutCoin::Deserialize(deserializer2));
    }

    //
    // JSON
    //
    {
        Json json(pegOutCoin.ToJSON());
        REQUIRE(json.GetKeys() == std::vector<std::string>({ "address", "amount" }));
        REQUIRE(json.GetRequired<uint64_t>("amount") == amount);
        REQUIRE(json.GetRequired<Bech32Address>("address") == address);

        REQUIRE(pegOutCoin == PegOutCoin::FromJSON(json));
    }

    //
    // Getters
    //
    {
        REQUIRE(pegOutCoin.GetAmount() == amount);
        REQUIRE(pegOutCoin.GetAddress() == address);
    }
}