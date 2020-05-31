#include <catch.hpp>

#include "TestUtil.h"

#include <mw/file/BitmapFile.h>
#include <mw/file/FileRemover.h>

TEST_CASE("BitmapFile")
{
    File tempFile = TestUtil::CreateTemp();
    FileRemover remover(tempFile);

    tempFile.Write({ 0x58 });
    auto pBitmap = BitmapFile::Load(tempFile);

    REQUIRE(!pBitmap->IsSet(0));
    REQUIRE(pBitmap->IsSet(1));
    REQUIRE(!pBitmap->IsSet(2));
    REQUIRE(pBitmap->IsSet(3));
    REQUIRE(pBitmap->IsSet(4));
    REQUIRE(!pBitmap->IsSet(5));
    REQUIRE(!pBitmap->IsSet(6));
    REQUIRE(!pBitmap->IsSet(7));
}