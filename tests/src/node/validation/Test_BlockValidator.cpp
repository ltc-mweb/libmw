#include <catch.hpp>

#include <mw/ltc/node/Context.h>
#include <mw/ltc/node/validation/BlockValidator.h>

TEST_CASE("BlockValidator")
{
    BlockValidator validator(Context::Create());

    //auto pHeader = std::make_shared<Header>(0, Hash::ValueOf(0), Hash::ValueOf(0), Hash::ValueOf(0))
    //auto pBlock = std::make_shared<Block>()
}