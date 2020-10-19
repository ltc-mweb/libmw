#include <catch.hpp>

#include <mw/db/LeafDB.h>

#include <test_framework/DBWrapper.h>

TEST_CASE("LeafDB")
{
    auto pDatabase = std::make_shared<TestDBWrapper>();
    LeafDB ldb(pDatabase.get());

    auto leaf1 = mmr::Leaf::Create(mmr::LeafIndex::At(0), { 0, 1, 2 });
    auto leaf2 = mmr::Leaf::Create(mmr::LeafIndex::At(1), { 1, 2, 3 });
    auto leaf3 = mmr::Leaf::Create(mmr::LeafIndex::At(2), { 2, 3, 4 });

    ldb.Add({leaf1, leaf2, leaf3});

    auto pLeaf = ldb.Get(mmr::LeafIndex::At(0), mw::Hash(leaf1.GetHash()));
    REQUIRE(pLeaf->GetHash() == leaf1.GetHash());
    REQUIRE(pLeaf->vec() == leaf1.vec());

    pLeaf = ldb.Get(mmr::LeafIndex::At(1), mw::Hash(leaf2.GetHash()));
    REQUIRE(pLeaf->GetHash() == leaf2.GetHash());
    REQUIRE(pLeaf->vec() == leaf2.vec());

    pLeaf = ldb.Get(mmr::LeafIndex::At(2), mw::Hash(leaf3.GetHash()));
    REQUIRE(pLeaf->GetHash() == leaf3.GetHash());
    REQUIRE(pLeaf->vec() == leaf3.vec());

    std::vector<uint8_t> data;
    REQUIRE(pDatabase->Read("L" + leaf1.GetHash().ToHex(), data));
    REQUIRE(data == leaf1.vec());
    REQUIRE(pDatabase->Read("L" + leaf2.GetHash().ToHex(), data));
    REQUIRE(data == leaf2.vec());
    REQUIRE(pDatabase->Read("L" + leaf3.GetHash().ToHex(), data));
    REQUIRE(data == leaf3.vec());

    ldb.Remove({leaf2.GetHash()});
    REQUIRE(ldb.Get(mmr::LeafIndex::At(1), mw::Hash(leaf2.GetHash())) == nullptr);

    ldb.RemoveAll();
    REQUIRE(ldb.Get(mmr::LeafIndex::At(0), mw::Hash(leaf1.GetHash())) == nullptr);
    REQUIRE(ldb.Get(mmr::LeafIndex::At(2), mw::Hash(leaf3.GetHash())) == nullptr);

    ldb.Add({leaf2});
    pLeaf = ldb.Get(mmr::LeafIndex::At(1), mw::Hash(leaf2.GetHash()));
    REQUIRE(pLeaf->GetHash() == leaf2.GetHash());
    REQUIRE(pLeaf->vec() == leaf2.vec());
}