#include <catch.hpp>

#include <mw/crypto/Random.h>
#include <mw/db/VectorDB.h>
#include <mw/serialization/Serializer.h>

#include <test_framework/DBWrapper.h>

std::vector<uint8_t> itov(uint64_t i)
{
    return Serializer().Append(i).vec();
}

std::string itos(uint64_t i)
{
    auto v = itov(i);
    return std::string(v.begin(), v.end());
}

TEST_CASE("VectorDB")
{
    auto pDatabase = std::make_shared<TestDBWrapper>();
    VectorDB vdb("test", pDatabase.get());

    REQUIRE(vdb.Size() == 0);
    REQUIRE(vdb.Get(0) == nullptr);

    SecretKey key1 = Random::CSPRNG<32>();
    SecretKey key2 = Random::CSPRNG<32>();
    SecretKey64 key3 = Random::CSPRNG<64>();

    vdb.Add({key1.vec()});
    REQUIRE(vdb.Size() == 1);
    REQUIRE(*vdb.Get(0) == key1.vec());
    REQUIRE(vdb.Get(1) == nullptr);

    vdb.Add({key2.vec(), key3.vec()});
    REQUIRE(vdb.Size() == 3);
    REQUIRE(*vdb.Get(0) == key1.vec());
    REQUIRE(*vdb.Get(1) == key2.vec());
    REQUIRE(*vdb.Get(2) == key3.vec());

    std::vector<uint8_t> data;
    REQUIRE(pDatabase->Read("VtestSize", data));
    REQUIRE(data == itov(3));
    REQUIRE(pDatabase->Read("Vtest" + itos(0), data));
    REQUIRE(data == key1.vec());
    REQUIRE(pDatabase->Read("Vtest" + itos(1), data));
    REQUIRE(data == key2.vec());
    REQUIRE(pDatabase->Read("Vtest" + itos(2), data));
    REQUIRE(data == key3.vec());

    vdb.RemoveAt({1});
    REQUIRE(vdb.Size() == 3);
    REQUIRE(*vdb.Get(0) == key1.vec());
    REQUIRE(vdb.Get(1) == nullptr);
    REQUIRE(*vdb.Get(2) == key3.vec());

    vdb.Rewind(1);
    REQUIRE(vdb.Size() == 1);
    REQUIRE(*vdb.Get(0) == key1.vec());
    REQUIRE(vdb.Get(1) == nullptr);
    REQUIRE(vdb.Get(2) == nullptr);

    vdb.RemoveAll();
    REQUIRE(vdb.Size() == 0);
    REQUIRE(vdb.Get(0) == nullptr);
}