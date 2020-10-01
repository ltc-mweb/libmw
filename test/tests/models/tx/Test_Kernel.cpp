#include <catch.hpp>

#include <mw/crypto/Crypto.h>
#include <mw/crypto/Random.h>
#include <mw/models/tx/Kernel.h>

TEST_CASE("Plain Kernel")
{
    uint64_t fee = 1000;
    Commitment excess(Random::CSPRNG<33>().GetBigInt());
    Signature signature(Random::CSPRNG<64>().GetBigInt());
    Kernel kernel = Kernel::CreatePlain(fee, Commitment(excess), Signature(signature));

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = kernel.Serialized();
        REQUIRE(serialized.size() == 106);

        Deserializer deserializer(serialized);
        REQUIRE(deserializer.Read<uint8_t>() == 0);
        REQUIRE(deserializer.Read<uint64_t>() == fee);
        REQUIRE(Commitment::Deserialize(deserializer) == excess);
        REQUIRE(Signature::Deserialize(deserializer) == signature);

        Deserializer deserializer2(serialized);
        REQUIRE(kernel == Kernel::Deserialize(deserializer2));
    }

    //
    // JSON
    //
    {
        Json json(kernel.ToJSON());
        REQUIRE(json.GetKeys() == std::vector<std::string>({ "excess", "fee", "signature", "type" }));
        REQUIRE(json.GetRequired<Commitment>("excess") == excess);
        REQUIRE(json.GetRequired<uint64_t>("fee") == fee);
        REQUIRE(json.GetRequired<Signature>("signature") == signature);
        REQUIRE(json.GetRequired<std::string>("type") == "PLAIN");

        REQUIRE(kernel == Kernel::FromJSON(json));
    }

    //
    // Signature Message
    //
    {
        mw::Hash hashed = kernel.GetSignatureMessage();
        std::vector<uint8_t> message = Serializer()
            .Append<uint8_t>(0)
            .Append<uint64_t>(fee)
            .vec();
        REQUIRE(hashed == Hashed(message));
    }

    //
    // Getters
    //
    {
        REQUIRE(!kernel.IsPegIn());
        REQUIRE(!kernel.IsPegOut());
        REQUIRE(kernel.GetPeggedIn() == 0);
        REQUIRE(kernel.GetPeggedOut() == 0);
        REQUIRE(kernel.GetLockHeight() == 0);
        REQUIRE(kernel.GetFee() == fee);
        REQUIRE(kernel.GetCommitment() == excess);
        REQUIRE(kernel.GetSignature() == signature);
    }
}

TEST_CASE("Peg-In Kernel")
{
    uint64_t amount = 50;
    Commitment excess(Random::CSPRNG<33>().GetBigInt());
    Signature signature(Random::CSPRNG<64>().GetBigInt());
    Kernel kernel = Kernel::CreatePegIn(amount, Commitment(excess), Signature(signature));

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = kernel.Serialized();
        REQUIRE(serialized.size() == 106);

        Deserializer deserializer(serialized);
        REQUIRE(deserializer.Read<uint8_t>() == 1);
        REQUIRE(deserializer.Read<uint64_t>() == amount);
        REQUIRE(Commitment::Deserialize(deserializer) == excess);
        REQUIRE(Signature::Deserialize(deserializer) == signature);

        Deserializer deserializer2(serialized);
        REQUIRE(kernel == Kernel::Deserialize(deserializer2));
    }

    //
    // JSON
    //
    {
        Json json(kernel.ToJSON());
        REQUIRE(json.GetKeys() == std::vector<std::string>({ "amount", "excess", "signature", "type" }));
        REQUIRE(json.GetRequired<uint64_t>("amount") == amount);
        REQUIRE(json.GetRequired<Commitment>("excess") == excess);
        REQUIRE(json.GetRequired<Signature>("signature") == signature);
        REQUIRE(json.GetRequired<std::string>("type") == "PEGIN");

        REQUIRE(kernel == Kernel::FromJSON(json));
    }

    //
    // Signature Message
    //
    {
        mw::Hash hashed = kernel.GetSignatureMessage();
        std::vector<uint8_t> message = Serializer()
            .Append<uint8_t>(1)
            .Append<uint64_t>(amount)
            .vec();
        REQUIRE(hashed == Hashed(message));
    }

    //
    // Getters
    //
    {
        REQUIRE(kernel.IsPegIn());
        REQUIRE(!kernel.IsPegOut());
        REQUIRE(kernel.GetPeggedIn() == amount);
        REQUIRE(kernel.GetPeggedOut() == 0);
        REQUIRE(kernel.GetLockHeight() == 0);
        REQUIRE(kernel.GetFee() == 0);
        REQUIRE(kernel.GetCommitment() == excess);
        REQUIRE(kernel.GetSignature() == signature);
    }
}

TEST_CASE("Peg-Out Kernel")
{
    uint64_t amount = 50;
    uint64_t fee = 1000;
    Bech32Address address = Bech32Address::FromString("bc1qc7slrfxkknqcq2jevvvkdgvrt8080852dfjewde450xdlk4ugp7szw5tk9");
    Commitment excess(Random::CSPRNG<33>().GetBigInt());
    Signature signature(Random::CSPRNG<64>().GetBigInt());
    Kernel kernel = Kernel::CreatePegOut(amount, fee, Bech32Address(address), Commitment(excess), Signature(signature));

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = kernel.Serialized();
        REQUIRE(serialized.size() == 168);

        Deserializer deserializer(serialized);
        REQUIRE(deserializer.Read<uint8_t>() == 2);
        REQUIRE(deserializer.Read<uint64_t>() == fee);
        REQUIRE(deserializer.Read<uint64_t>() == amount);
        REQUIRE(Bech32Address::Deserialize(deserializer) == address);
        REQUIRE(Commitment::Deserialize(deserializer) == excess);
        REQUIRE(Signature::Deserialize(deserializer) == signature);

        Deserializer deserializer2(serialized);
        REQUIRE(kernel == Kernel::Deserialize(deserializer2));
    }

    //
    // JSON
    //
    {
        Json json(kernel.ToJSON());
        REQUIRE(json.GetKeys() == std::vector<std::string>({ "address", "amount", "excess", "fee", "signature", "type" }));
        REQUIRE(json.GetRequired<uint64_t>("amount") == amount);
        REQUIRE(json.GetRequired<uint64_t>("fee") == fee);
        REQUIRE(json.GetRequired<Bech32Address>("address") == address);
        REQUIRE(json.GetRequired<Commitment>("excess") == excess);
        REQUIRE(json.GetRequired<Signature>("signature") == signature);
        REQUIRE(json.GetRequired<std::string>("type") == "PEGOUT");

        REQUIRE(kernel == Kernel::FromJSON(json));
    }

    //
    // Signature Message
    //
    {
        mw::Hash hashed = kernel.GetSignatureMessage();
        std::vector<uint8_t> message = Serializer()
            .Append<uint8_t>(2)
            .Append<uint64_t>(fee)
            .Append<uint64_t>(amount)
            .Append(address)
            .vec();
        REQUIRE(hashed == Hashed(message));
    }

    //
    // Getters
    //
    {
        REQUIRE(!kernel.IsPegIn());
        REQUIRE(kernel.IsPegOut());
        REQUIRE(kernel.GetPeggedIn() == 0);
        REQUIRE(kernel.GetPeggedOut() == amount);
        REQUIRE(kernel.GetLockHeight() == 0);
        REQUIRE(kernel.GetFee() == fee);
        REQUIRE(kernel.GetCommitment() == excess);
        REQUIRE(kernel.GetSignature() == signature);
    }
}

TEST_CASE("Height-Locked")
{
    uint64_t fee = 1000;
    uint64_t lockHeight = 2500;
    Commitment excess(Random::CSPRNG<33>().GetBigInt());
    Signature signature(Random::CSPRNG<64>().GetBigInt());
    Kernel kernel = Kernel::CreateHeightLocked(fee, lockHeight, Commitment(excess), Signature(signature));

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = kernel.Serialized();
        REQUIRE(serialized.size() == 114);

        Deserializer deserializer(serialized);
        REQUIRE(deserializer.Read<uint8_t>() == 3);
        REQUIRE(deserializer.Read<uint64_t>() == fee);
        REQUIRE(deserializer.Read<uint64_t>() == lockHeight);
        REQUIRE(Commitment::Deserialize(deserializer) == excess);
        REQUIRE(Signature::Deserialize(deserializer) == signature);

        Deserializer deserializer2(serialized);
        REQUIRE(kernel == Kernel::Deserialize(deserializer2));
    }

    //
    // JSON
    //
    {
        Json json(kernel.ToJSON());
        REQUIRE(json.GetKeys() == std::vector<std::string>({ "excess", "fee", "lock_height", "signature", "type" }));
        REQUIRE(json.GetRequired<uint64_t>("fee") == fee);
        REQUIRE(json.GetRequired<uint64_t>("lock_height") == lockHeight);
        REQUIRE(json.GetRequired<Commitment>("excess") == excess);
        REQUIRE(json.GetRequired<Signature>("signature") == signature);
        REQUIRE(json.GetRequired<std::string>("type") == "HEIGHT_LOCKED");

        REQUIRE(kernel == Kernel::FromJSON(json));
    }

    //
    // Signature Message
    //
    {
        mw::Hash hashed = kernel.GetSignatureMessage();
        std::vector<uint8_t> message = Serializer()
            .Append<uint8_t>(3)
            .Append<uint64_t>(fee)
            .Append<uint64_t>(lockHeight)
            .vec();
        REQUIRE(hashed == Hashed(message));
    }

    //
    // Getters
    //
    {
        REQUIRE(!kernel.IsPegIn());
        REQUIRE(!kernel.IsPegOut());
        REQUIRE(kernel.GetPeggedIn() == 0);
        REQUIRE(kernel.GetPeggedOut() == 0);
        REQUIRE(kernel.GetLockHeight() == lockHeight);
        REQUIRE(kernel.GetFee() == fee);
        REQUIRE(kernel.GetCommitment() == excess);
        REQUIRE(kernel.GetSignature() == signature);
    }
}

// TODO: Test unknown Kernel to ensure we can support soft-forking futre kernel types.