#include <catch.hpp>

#include <mw/node/CoinsView.h>
#include <mw/crypto/Hasher.h>
#include <mw/file/FileRemover.h>
#include <mw/mmr/backends/FileBackend.h>

#include <test_framework/TestUtil.h>

TEST_CASE("mw::CoinsViewDB")
{
    FilePath chain_path = test::TestUtil::GetTempDir();
    FileRemover remover(chain_path); // Removes the directory when this goes out of scope.

    auto mmrPath = chain_path.GetChild("outputs");
    auto pBackend = mmr::FileBackend::Open(mmrPath, boost::optional<uint16_t>(34));
    mmr::MMR::Ptr pMMR = std::make_shared<mmr::MMR>(pBackend);
}