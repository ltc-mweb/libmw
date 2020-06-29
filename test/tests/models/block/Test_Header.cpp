#include <catch.hpp>

#include <mw/models/block/Header.h>

TEST_CASE("Header")
{
	const uint16_t version = 1;
	const uint64_t height = 2;
	const int64_t timestamp = 3;

	mw::Hash previousBlockHash = mw::Hash::FromHex("0x0102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F");
	mw::Hash previousRoot = mw::Hash::FromHex("0x1102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F");
	mw::Hash outputRoot = mw::Hash::FromHex("0x2102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F");
	mw::Hash rangeProofRoot = mw::Hash::FromHex("0x3102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F");
	mw::Hash kernelRoot = mw::Hash::FromHex("0x4102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F");
	mw::Hash totalKernelOffset = mw::Hash::FromHex("0x5102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F");

	const uint64_t outputMMRSize = 4;
	const uint64_t kernelMMRSize = 5;

	// TODO: Finish this
}