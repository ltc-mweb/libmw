#include <catch.hpp>

#include <mw/models/block/Header.h>

TEST_CASE("Header")
{
    const uint64_t height = 1;
    const uint64_t outputMMRSize = 2;
    const uint64_t kernelMMRSize = 3;

    mw::Hash outputRoot = mw::Hash::FromHex("000102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F20");
    mw::Hash rangeProofRoot = mw::Hash::FromHex("001102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F20");
    mw::Hash kernelRoot = mw::Hash::FromHex("002102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F20");
    mw::Hash leafsetRoot = mw::Hash::FromHex("003102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F20");
    BlindingFactor kernelOffset = BigInt<32>::FromHex("004102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F20");
    BlindingFactor ownerOffset = BigInt<32>::FromHex("005102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F20");

    mw::Header header(
        height,
        mw::Hash(outputRoot),
        mw::Hash(rangeProofRoot),
        mw::Hash(kernelRoot),
        mw::Hash(leafsetRoot),
        BlindingFactor(kernelOffset),
        BlindingFactor(ownerOffset),
        outputMMRSize,
        kernelMMRSize
    );
    mw::Header header2(
        height + 1,
        mw::Hash(outputRoot),
        mw::Hash(rangeProofRoot),
        mw::Hash(kernelRoot),
        mw::Hash(leafsetRoot),
        BlindingFactor(kernelOffset),
        BlindingFactor(ownerOffset),
        outputMMRSize,
        kernelMMRSize
    );

    REQUIRE_FALSE(header == header2);
    REQUIRE(header.GetHeight() == height);
    REQUIRE(header.GetOutputRoot() == outputRoot);
    REQUIRE(header.GetRangeProofRoot() == rangeProofRoot);
    REQUIRE(header.GetKernelRoot() == kernelRoot);
    REQUIRE(header.GetLeafsetRoot() == leafsetRoot);
    REQUIRE(header.GetKernelOffset() == kernelOffset);
    REQUIRE(header.GetOwnerOffset() == ownerOffset);
    REQUIRE(header.GetNumTXOs() == outputMMRSize);
    REQUIRE(header.GetNumKernels() == kernelMMRSize);
    REQUIRE(header.Format() == "66e2742e1967b7d0b95963014cb08ebe56e847e7b8d4dea8804651371cc5a411");

    Deserializer deserializer = header.Serialized();
    REQUIRE(header == mw::Header::Deserialize(deserializer));
    REQUIRE(header == mw::Header::FromJSON(header.ToJSON()));

    // TODO: Test JSON fields
}