#include <catch.hpp>

#include <mw/mmr/MMRUtil.h>
#include <unordered_set>

using namespace mmr;

#define REQUIRE_NEXT(iter, expected_pos) \
    REQUIRE(iter.Next()); \
    REQUIRE(iter.GetPosition() == expected_pos);

TEST_CASE("mmr::SiblingIter")
{
    // Height 0
    {
        SiblingIter iter(0, mmr::Index::At(22));
        REQUIRE_NEXT(iter, 0);
        REQUIRE_NEXT(iter, 1);
        REQUIRE_NEXT(iter, 3);
        REQUIRE_NEXT(iter, 4);
        REQUIRE_NEXT(iter, 7);
        REQUIRE_NEXT(iter, 8);
        REQUIRE_NEXT(iter, 10);
        REQUIRE_NEXT(iter, 11);
        REQUIRE_NEXT(iter, 15);
        REQUIRE_NEXT(iter, 16);
        REQUIRE_NEXT(iter, 18);
        REQUIRE_NEXT(iter, 19);
        REQUIRE_NEXT(iter, 22);
        REQUIRE_FALSE(iter.Next());
    }

    // Height 1
    {
        SiblingIter iter(1, mmr::Index::At(84));
        REQUIRE_NEXT(iter, 2);
        REQUIRE_NEXT(iter, 5);
        REQUIRE_NEXT(iter, 9);
        REQUIRE_NEXT(iter, 12);
        REQUIRE_NEXT(iter, 17);
        REQUIRE_NEXT(iter, 20);
        REQUIRE_NEXT(iter, 24);
        REQUIRE_NEXT(iter, 27);
        REQUIRE_NEXT(iter, 33);
        REQUIRE_NEXT(iter, 36);
        REQUIRE_NEXT(iter, 40);
        REQUIRE_NEXT(iter, 43);
        REQUIRE_NEXT(iter, 48);
        REQUIRE_NEXT(iter, 51);
        REQUIRE_NEXT(iter, 55);
        REQUIRE_NEXT(iter, 58);
        REQUIRE_NEXT(iter, 65);
        REQUIRE_NEXT(iter, 68);
        REQUIRE_NEXT(iter, 72);
        REQUIRE_NEXT(iter, 75);
        REQUIRE_NEXT(iter, 80);
        REQUIRE_NEXT(iter, 83);
        REQUIRE_FALSE(iter.Next());
    }

    // Height 2
    {
        SiblingIter iter(2, mmr::Index::At(100));
        REQUIRE_NEXT(iter, 6);
        REQUIRE_NEXT(iter, 13);
        REQUIRE_NEXT(iter, 21);
        REQUIRE_NEXT(iter, 28);
        REQUIRE_NEXT(iter, 37);
        REQUIRE_NEXT(iter, 44);
        REQUIRE_NEXT(iter, 52);
        REQUIRE_NEXT(iter, 59);
        REQUIRE_NEXT(iter, 69);
        REQUIRE_NEXT(iter, 76);
        REQUIRE_NEXT(iter, 84);
        REQUIRE_NEXT(iter, 91);
        REQUIRE_NEXT(iter, 100);
        REQUIRE_FALSE(iter.Next());
    }

    // Thorough check of positions 0-2500
    {
        mmr::Index last_node = mmr::Index::At(2500);
        std::unordered_set<uint64_t> nodes_found;
        for (uint8_t height = 0; height <= 12; height++) {
            SiblingIter iter(height, last_node);

            while (iter.Next()) {
                REQUIRE(nodes_found.count(iter.GetPosition()) == 0);
                REQUIRE(mmr::Index::At(iter.GetPosition()).GetHeight() == height);
                nodes_found.insert(iter.GetPosition());
            }
        }

        for (uint64_t i = 0; i <= last_node.GetPosition(); i++) {
            REQUIRE(nodes_found.count(i) == 1);
        }
    }
}