#include <catch.hpp>

#include <test_framework/TestWallet.h>
#include <mw/wallet/Wallet.h>
#include <libmw/libmw.h>

TEST_CASE("Wallet::GetStealthAddress")
{
    libmw::IWallet::Ptr pWallet = std::make_shared<TestWallet>(BlindingFactor::FromHex("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"));

    mw::Hash a(pWallet->GetHDKey("m/1/0/100'").keyBytes);
    REQUIRE(a.ToHex() == "6a94ffabf7cb6c56b7f1bda50158019a49d101e270ad202e04912b2f001ece79");

    mw::Hash b(pWallet->GetHDKey("m/1/0/101'").keyBytes);
    REQUIRE(b.ToHex() == "da5b685cbcdad4aabcec8f58253f8f4aa89a116659b1e5a8e3c407cc09c19738");

    StealthAddress address_100 = Wallet::Open(pWallet).GetStealthAddress(100);
    REQUIRE(address_100.A().Format() == "02676d6f55a58297072c602b9e3c98bdcf7f2632c4c88686bd8e8cf3f12d394111");
    REQUIRE(address_100.B().Format() == "03e7421b2c09154a4201072c733929dfc25262dd79146f1164b4b000c4f5533e01");
}