#include <catch.hpp>

#include <mw/mmr/LeafSet.h>
#include <mw/crypto/Hasher.h>
#include <mw/file/FileRemover.h>

#include <test_framework/TestUtil.h>

TEST_CASE("mmr::LeafSet")
{
	File file(test::TestUtil::GetTempDir().GetChild("LeafSet.bin"));
	FileRemover remover(file); // Removes the file when this goes out of scope.
	file.Create();

	mmr::LeafSet leafset(MemMap{ file });

	REQUIRE(leafset.GetSize() == 0);
	REQUIRE_FALSE(leafset.Contains(mmr::LeafIndex::At(0)));
	REQUIRE_FALSE(leafset.Contains(mmr::LeafIndex::At(1)));
	REQUIRE_FALSE(leafset.Contains(mmr::LeafIndex::At(2)));
	REQUIRE(leafset.Root(0) == Hashed(std::vector<uint8_t>{ }));

	leafset.Add(mmr::LeafIndex::At(0));
	REQUIRE(leafset.Contains(mmr::LeafIndex::At(0)));
	REQUIRE(leafset.GetSize() == 8);
	REQUIRE(leafset.Root(1) == Hashed({ 0b10000000 }));
	REQUIRE(leafset.Root(2) == Hashed({ 0b10000000 }));
	REQUIRE(leafset.Root(3) == Hashed({ 0b10000000 }));

	leafset.Add(mmr::LeafIndex::At(1));
	REQUIRE(leafset.Contains(mmr::LeafIndex::At(1)));
	REQUIRE(leafset.GetSize() == 8);
	REQUIRE(leafset.Root(1) == Hashed({ 0b10000000 }));
	REQUIRE(leafset.Root(2) == Hashed({ 0b11000000 }));
	REQUIRE(leafset.Root(3) == Hashed({ 0b11000000 }));

	leafset.Add(mmr::LeafIndex::At(2));
	REQUIRE(leafset.Contains(mmr::LeafIndex::At(2)));
	REQUIRE(leafset.GetSize() == 8);
	REQUIRE(leafset.Root(1) == Hashed({ 0b10000000 }));
	REQUIRE(leafset.Root(2) == Hashed({ 0b11000000 }));
	REQUIRE(leafset.Root(3) == Hashed({ 0b11100000 }));

	leafset.Remove(mmr::LeafIndex::At(1));
	REQUIRE_FALSE(leafset.Contains(mmr::LeafIndex::At(1)));
	REQUIRE(leafset.Root(1) == Hashed({ 0b10000000 }));
	REQUIRE(leafset.Root(2) == Hashed({ 0b10000000 }));
	REQUIRE(leafset.Root(3) == Hashed({ 0b10100000 }));

	leafset.Rewind(2, { mmr::LeafIndex::At(1) });
	REQUIRE(leafset.Root(1) == Hashed({ 0b10000000 }));
	REQUIRE(leafset.Root(2) == Hashed({ 0b11000000 }));
	REQUIRE(leafset.Root(3) == Hashed({ 0b11000000 }));
}