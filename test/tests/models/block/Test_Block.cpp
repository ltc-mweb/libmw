#include <catch.hpp>

#include <mw/models/block/Block.h>
#include <mw/consensus/Aggregation.h>
#include <test_framework/models/Tx.h>

TEST_CASE("Block")
{
    test::Tx tx1 = test::Tx::CreatePegIn(10);
    test::Tx tx2 = test::Tx::CreatePegIn(20);
    mw::Transaction::CPtr pTransaction = Aggregation::Aggregate({
        tx1.GetTransaction(),
        tx2.GetTransaction()
    });

    mw::Header::CPtr pHeader = std::make_shared<mw::Header>(
        100,
        mw::Hash::FromHex("000102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F20"),
        mw::Hash::FromHex("001102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F20"),
        mw::Hash::FromHex("002102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F20"),
        mw::Hash::FromHex("003102030405060708090A0B0C0D0E0F1112131415161718191A1B1C1D1E1F20"),
        BlindingFactor(pTransaction->GetOffset()),
        pTransaction->GetOutputs().size(),
        pTransaction->GetKernels().size()
    );

    mw::Block block(pHeader, pTransaction->GetBody());

    REQUIRE(*block.GetHeader() == *pHeader);
    REQUIRE(block.GetInputs() == pTransaction->GetInputs());
    REQUIRE(block.GetOutputs() == pTransaction->GetOutputs());
    REQUIRE(block.GetKernels() == pTransaction->GetKernels());
    REQUIRE(block.GetHeight() == pHeader->GetHeight());
    REQUIRE(block.GetOffset() == pHeader->GetOffset());

    REQUIRE(block.GetPegInKernels() == pTransaction->GetKernels());
    REQUIRE(block.GetPegInAmount() == 30);
    REQUIRE(block.GetPegOutKernels().empty());

    Deserializer deserializer = block.Serialized();
    mw::Block block2 = mw::Block::Deserialize(deserializer);
    REQUIRE(*block.GetHeader() == *block2.GetHeader());
    REQUIRE(block.GetTxBody() == block2.GetTxBody());

    block2 = mw::Block::FromJSON(block.ToJSON());
    REQUIRE(*block.GetHeader() == *block2.GetHeader());
    REQUIRE(block.GetTxBody() == block2.GetTxBody());

    REQUIRE_FALSE(block.WasValidated());
    block.Validate();
    block.MarkAsValidated();
    REQUIRE(block.WasValidated());
}