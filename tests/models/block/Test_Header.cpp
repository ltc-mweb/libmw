#include <catch.hpp>

#include <mw/ltc/models/block/Header.h>

TEST_CASE("Header")
{
	const uint16_t version = 1;
	const uint64_t height = 2;
	const int64_t timestamp = 3;

	Hash previousBlockHash = Hash::FromHex("0x0102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F");
	Hash previousRoot = Hash::FromHex("0x1102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F");
	Hash outputRoot = Hash::FromHex("0x2102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F");
	Hash rangeProofRoot = Hash::FromHex("0x3102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F");
	Hash kernelRoot = Hash::FromHex("0x4102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F");
	Hash totalKernelOffset = Hash::FromHex("0x5102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F");

	const uint64_t outputMMRSize = 4;
	const uint64_t kernelMMRSize = 5;

	// TODO: Finish this
}