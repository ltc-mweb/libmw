#include <catch.hpp>

#include <mw/models/crypto/BigInteger.h>

TEST_CASE("BigInt")
{
    BigInt<8> bigInt1 = BigInt<8>::FromHex("0123456789AbCdEf");
    REQUIRE(bigInt1.ToHex() == "0123456789abcdef");

    BigInt<4> bigInt2 = BigInt<4>::Max();
    REQUIRE(bigInt2.ToHex() == "ffffffff");

    BigInt<8> bigInt3 = BigInt<8>::ValueOf(12); // TODO: Pass in uint64_t
    REQUIRE(bigInt3.ToHex() == "000000000000000c");
    // TODO: Finish
}