#include <catch.hpp>

#include <test_framework/TestWallet.h>
#include <mw/wallet/Wallet.h>
#include <libmw/libmw.h>

TEST_CASE("Wallet::GetStealthAddress")
{
    libmw::IWallet::Ptr pWallet = std::make_shared<TestWallet>(BlindingFactor::FromHex("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"));

    mw::Hash a(pWallet->GetHDKey("m/1/0/100'").keyBytes);
    REQUIRE(a.ToHex() == "c8d18a6029c163835157be82d4d0c4c90534a95b2f6f2a5126c6928b95c59a0c");

    mw::Hash b(pWallet->GetHDKey("m/1/0/101'").keyBytes);
    REQUIRE(b.ToHex() == "41f4ea4a4cfc48cf28f4100b9bc37214d80fcce262293ba64685219def319519");

    StealthAddress address_100 = Wallet::Open(pWallet).GetStealthAddress(100);
    REQUIRE(address_100.A().Format() == "022d0e21f6caa1323cafc266c4f2423808d3536cde69036a02fa3bb94f00734e03");
    REQUIRE(address_100.B().Format() == "03321465476a81ac478588e084bb4806e76c0b28e4e067edaf28fbf82209dbb17a");
}