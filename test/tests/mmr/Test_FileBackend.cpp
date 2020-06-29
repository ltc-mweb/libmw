#include <catch.hpp>

#include <mw/mmr/backends/FileBackend.h>
#include <mw/models/tx/Kernel.h>
#include <mw/crypto/Random.h>
#include <mw/file/FileRemover.h>

using namespace mmr;

static FilePath CreateTempDir()
{
    return FilePath(fs::temp_directory_path() / (StringUtil::ToWide(Random::CSPRNG<6>().GetBigInt().ToHex()) + L"\u30c4"));
}

TEST_CASE("mmr::FileBackend")
{
    FilePath tempDir = CreateTempDir();
    FileRemover remover(tempDir);

    {
        auto pBackend = FileBackend::Open(tempDir, boost::none);
        pBackend->AddLeaf(mmr::Leaf::Create(mmr::LeafIndex::At(0), { 0x05, 0x03, 0x07 }));
        pBackend->Commit();
    }
    {
        auto pBackend = FileBackend::Open(tempDir, boost::none);
        REQUIRE(pBackend->GetNumLeaves() == 1);
    }
}