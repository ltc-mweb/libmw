#include <catch.hpp>

#include <mw/mmr/backends/FileBackend.h>
#include <mw/models/tx/Kernel.h>
#include <mw/crypto/Random.h>
#include <mw/file/ScopedFileRemover.h>

#include <test_framework/DBWrapper.h>
#include <test_framework/TestUtil.h>

using namespace mmr;

static FilePath CreateTempDir()
{
    return FilePath(filesystem::temp_directory_path() / (StringUtil::ToWide(Random::CSPRNG<6>().GetBigInt().ToHex()) + L"\u30c4"));
}

TEST_CASE("mmr::FileBackend")
{
    FilePath tempDir = test::TestUtil::GetTempDir();// CreateTempDir();
    ScopedFileRemover remover(tempDir);
    auto pDatabase = std::make_shared<TestDBWrapper>();

    {
        auto pBackend = FileBackend::Open('T', tempDir, pDatabase);
        pBackend->AddLeaf(mmr::Leaf::Create(mmr::LeafIndex::At(0), { 0x05, 0x03, 0x07 }));
        pBackend->Commit();
    }
    {
        auto pBackend = FileBackend::Open('T', tempDir, pDatabase);
        REQUIRE(pBackend->GetNumLeaves() == 1);
        auto leaf = pBackend->GetLeaf(mmr::LeafIndex::At(0));
        REQUIRE(leaf.vec() == std::vector<uint8_t>{ 0x05, 0x03, 0x07 });
    }
}