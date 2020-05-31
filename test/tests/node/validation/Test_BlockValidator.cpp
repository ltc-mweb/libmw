#include <catch.hpp>

#include <mw/node/validation/BlockValidator.h>
#include <test_framework/Chain.h>

TEST_CASE("BlockValidator")
{
    test::Node::Ptr pNode = std::make_shared<test::Node>();
    test::TestChain chain(pNode);

    BlockValidator validator;

    //auto pHeader = std::make_shared<Header>(0, Hash::ValueOf(0), Hash::ValueOf(0), Hash::ValueOf(0))
    //auto pBlock = std::make_shared<Block>()
}