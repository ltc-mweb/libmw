#include <catch.hpp>

#include <mw/mmr/MMR.h>
#include <mw/mmr/backends/VectorBackend.h>

using namespace mmr;

TEST_CASE("mmr::MMR")
{
    auto pBackend = std::make_shared<VectorBackend>();
    MMR mmr(pBackend);

    std::vector<uint8_t> leaf0({ 0, 1, 2 });
    std::vector<uint8_t> leaf1({ 1, 2, 3 });
    std::vector<uint8_t> leaf2({ 2, 3, 4 });
    std::vector<uint8_t> leaf3({ 3, 4, 5 });
    std::vector<uint8_t> leaf4({ 4, 5, 6 });

    mmr.Add(leaf0);
    mmr.Add(leaf1);
    mmr.Add(leaf2);
    mmr.Add(leaf3);

    REQUIRE(mmr.Get(LeafIndex::At(0)) == Leaf::Create(LeafIndex::At(0), std::vector<uint8_t>(leaf0)));
    REQUIRE(mmr.Get(LeafIndex::At(1)) == Leaf::Create(LeafIndex::At(1), std::vector<uint8_t>(leaf1)));
    REQUIRE(mmr.Get(LeafIndex::At(2)) == Leaf::Create(LeafIndex::At(2), std::vector<uint8_t>(leaf2)));
    REQUIRE(mmr.Get(LeafIndex::At(3)) == Leaf::Create(LeafIndex::At(3), std::vector<uint8_t>(leaf3)));

    REQUIRE(mmr.GetNumNodes() == 7);
    REQUIRE(mmr.Root() == mw::Hash::FromHex("531b5c35f430f911db70cbf892cec23165d291bb08e568049f2bdc8174ded78a"));

    mmr.Add(leaf4);
    REQUIRE(mmr.Get(LeafIndex::At(4)) == Leaf::Create(LeafIndex::At(4), std::vector<uint8_t>(leaf4)));
    REQUIRE(mmr.GetNumNodes() == 8);
    REQUIRE(mmr.Root() == mw::Hash::FromHex("8479b8a461e696556726def136322f4b654986436a54c849e251b7a8a94bd62c"));

    mmr.Rewind(7);
    REQUIRE(mmr.GetNumNodes() == 7);
    REQUIRE(mmr.Root() == mw::Hash::FromHex("531b5c35f430f911db70cbf892cec23165d291bb08e568049f2bdc8174ded78a"));
}