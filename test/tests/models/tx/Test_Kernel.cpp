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

        Deserializer deserializer(serialized);
        REQUIRE(deserializer.Read<uint8_t>() == 0);
        REQUIRE(deserializer.Read<uint64_t>() == fee);
        REQUIRE(Commitment::Deserialize(deserializer) == excess);
        REQUIRE(Signature::Deserialize(deserializer) == signature);

        Deserializer deserializer2(serialized);
        REQUIRE(kernel == Kernel::Deserialize(deserializer2));
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

        Deserializer deserializer(serialized);
        REQUIRE(deserializer.Read<uint8_t>() == 1);
        REQUIRE(deserializer.Read<uint64_t>() == amount);
        REQUIRE(Commitment::Deserialize(deserializer) == excess);
        REQUIRE(Signature::Deserialize(deserializer) == signature);

        Deserializer deserializer2(serialized);
        REQUIRE(kernel == Kernel::Deserialize(deserializer2));
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

TEST_CASE("Unknown Kernel")
{
    uint8_t features = 99;
    uint64_t fee = 1000;
    Commitment excess(Random::CSPRNG<33>().GetBigInt());
    Signature signature(Random::CSPRNG<64>().GetBigInt());
    std::vector<uint8_t> extraData = { 1, 2, 3 };

    Kernel kernel(
        features,     // Unknown type
        fee,
        0,
        0,
        boost::none,
        std::vector<uint8_t>(extraData),
        Commitment(excess),
        Signature(signature)
    );

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = kernel.Serialized();

        Deserializer deserializer(serialized);
        REQUIRE(deserializer.Read<uint8_t>() == features);
        REQUIRE(deserializer.Read<uint64_t>() == fee);
        REQUIRE(deserializer.Read<uint8_t>() == extraData.size());
        REQUIRE(deserializer.ReadVector(extraData.size()) == extraData);
        REQUIRE(Commitment::Deserialize(deserializer) == excess);
        REQUIRE(Signature::Deserialize(deserializer) == signature);

        Deserializer deserializer2(serialized);
        REQUIRE(kernel == Kernel::Deserialize(deserializer2));
    }

    //
    // Signature Message
    //
    {
        mw::Hash hashed = kernel.GetSignatureMessage();
        std::vector<uint8_t> message = Serializer()
            .Append<uint8_t>(features)
            .Append<uint64_t>(fee)
            .Append(extraData)
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
        REQUIRE(kernel.GetExtraData() == extraData);
    }
}