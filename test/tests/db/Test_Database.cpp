#include <catch.hpp>

#include <test_framework/TestUtil.h>
#include <mw/db/IBlockDB.h>
#include <mw/crypto/Random.h>

static mw::Header::CPtr GenerateRandomHeader(const uint64_t height)
{
    return std::make_shared<mw::Header>(
        height,
        mw::Hash{ Random().CSPRNG<32>().GetBigInt() },
        mw::Hash{ Random().CSPRNG<32>().GetBigInt() },
        mw::Hash{ Random().CSPRNG<32>().GetBigInt() },
        BlindingFactor{ Random().CSPRNG<32>().GetBigInt() },
        Random().FastRandom(),
        Random().FastRandom()
    );
}

TEST_CASE("Database - Headers - Batch rollback")
{
    Locked<IBlockDB> blockDB = BlockDBFactory::Open(test::TestUtil::GetTempDir());

    auto pHeader0 = GenerateRandomHeader(0);
    auto pHeader1 = GenerateRandomHeader(1);
    auto pHeader2 = GenerateRandomHeader(2);

    std::vector<mw::Hash> hashes{
        pHeader0->GetHash(),
        pHeader1->GetHash(),
        pHeader2->GetHash(),
        Random().CSPRNG<32>().GetBigInt()
    };

    auto batch = blockDB.BatchWrite();
    batch->AddHeader(pHeader0);
    batch->AddHeaders({ pHeader1, pHeader2 });

    auto pFoundHeader0 = batch->GetHeaderByHash(pHeader0->GetHash());
    REQUIRE(pFoundHeader0 == pHeader0);

    auto pFoundHeader1 = batch->GetHeaderByHash(pHeader1->GetHash());
    REQUIRE(pFoundHeader1 == pHeader1);

    auto pFoundHeader2 = batch->GetHeaderByHash(pHeader2->GetHash());
    REQUIRE(pFoundHeader2 == pHeader2);

    std::vector<mw::Header::CPtr> headers = batch->GetHeadersByHash(hashes);
    REQUIRE(headers.size() == 3);
    REQUIRE(headers[0] == pHeader0);
    REQUIRE(headers[1] == pHeader1);
    REQUIRE(headers[2] == pHeader2);

    batch->Rollback();

    REQUIRE(batch->GetHeaderByHash(pHeader0->GetHash()) == nullptr);
    REQUIRE(batch->GetHeaderByHash(pHeader1->GetHash()) == nullptr);
    REQUIRE(batch->GetHeaderByHash(pHeader2->GetHash()) == nullptr);

    REQUIRE(batch->GetHeadersByHash(hashes).empty());
}

TEST_CASE("Database - Headers - Batch commit")
{
    Locked<IBlockDB> blockDB = BlockDBFactory::Open(test::TestUtil::GetTempDir());

    auto pHeader0 = GenerateRandomHeader(0);
    auto pHeader1 = GenerateRandomHeader(1);
    auto pHeader2 = GenerateRandomHeader(2);

    std::vector<mw::Hash> hashes{
        Random().CSPRNG<32>().GetBigInt(),
        pHeader0->GetHash(),
        pHeader1->GetHash(),
        pHeader2->GetHash()
    };

    auto batch = blockDB.BatchWrite();
    batch->AddHeader(pHeader0);
    batch->AddHeaders({ pHeader1, pHeader2 });

    auto pFoundHeader0 = batch->GetHeaderByHash(pHeader0->GetHash());
    REQUIRE(pFoundHeader0 == pHeader0);

    auto pFoundHeader1 = batch->GetHeaderByHash(pHeader1->GetHash());
    REQUIRE(pFoundHeader1 == pHeader1);

    auto pFoundHeader2 = batch->GetHeaderByHash(pHeader2->GetHash());
    REQUIRE(pFoundHeader2 == pHeader2);

    std::vector<mw::Header::CPtr> headers = batch->GetHeadersByHash(hashes);
    REQUIRE(headers.size() == 3);
    REQUIRE(headers[0] == pHeader0);
    REQUIRE(headers[1] == pHeader1);
    REQUIRE(headers[2] == pHeader2);

    batch->Commit();

    REQUIRE(batch->GetHeaderByHash(pHeader0->GetHash()) == pHeader0);
    REQUIRE(batch->GetHeaderByHash(pHeader1->GetHash()) == pHeader1);
    REQUIRE(batch->GetHeaderByHash(pHeader2->GetHash()) == pHeader2);

    REQUIRE(batch->GetHeadersByHash(hashes) == headers);
}

// TODO: Test blocks & UTXOs