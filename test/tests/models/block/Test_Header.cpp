#include <catch.hpp>

#include <mw/models/block/Header.h>

TEST_CASE("Header")
{
	const uint16_t version = 1;
	const uint64_t height = 2;
	const int64_t timestamp = 3;

	mw::Hash previousBlockHash = mw::Hash::FromHex("000102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F20");
	mw::Hash previousRoot = mw::Hash::FromHex("001102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F20");
	mw::Hash outputRoot = mw::Hash::FromHex("002102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F20");
	mw::Hash rangeProofRoot = mw::Hash::FromHex("003102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F20");
	mw::Hash kernelRoot = mw::Hash::FromHex("004102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F20");
	mw::Hash totalKernelOffset = mw::Hash::FromHex("005102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F20");

	const uint64_t outputMMRSize = 4;
	const uint64_t kernelMMRSize = 5;

	// TODO: Finish this
}