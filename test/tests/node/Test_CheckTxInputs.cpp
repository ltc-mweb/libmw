#include <catch.hpp>

#include <libmw/libmw.h>
#include <mw/node/CoinsView.h>
#include <mw/crypto/Hasher.h>
#include <mw/file/ScopedFileRemover.h>
#include <mw/mmr/backends/FileBackend.h>
#include <mw/node/INode.h>

#include <test_framework/DBWrapper.h>
#include <test_framework/Miner.h>
#include <test_framework/TestUtil.h>

TEST_CASE("CheckTxInputs")
{
    FilePath datadir = test::TestUtil::GetTempDir();
    ScopedFileRemover remover(datadir);

    auto pDatabase = std::make_shared<TestDBWrapper>();
    auto pNode = mw::InitializeNode(datadir, "test", nullptr, pDatabase);
    REQUIRE(pNode != nullptr);

    auto pDBView = pNode->GetDBView();
    auto pCachedView = libmw::CoinsViewRef{ std::make_shared<mw::CoinsViewCache>(pDBView) };

    test::Miner miner;
    uint64_t height = 100;

    // Mine pegin tx
    BlindingFactor outputBF;
    test::Tx tx1 = test::Tx::CreatePegIn2(1000, outputBF);
    auto block = miner.MineBlock(height, { tx1 });
    pNode->ValidateBlock(block.GetBlock(), block.GetHash(), { tx1.GetPegInCoin() }, {});
    pNode->ConnectBlock(block.GetBlock(), pCachedView.pCoinsView);
    height += mw::ChainParams::GetPegInMaturity();

    // Try to spend the pegin output
    const Output& output1 = tx1.GetOutputs().front();
    Input input1(output1.GetFeatures(), Commitment(output1.GetCommitment()));
    test::Tx tx2 = test::Tx::CreateSpend(input1, outputBF, 1000, 10);
    auto transaction = libmw::TxRef{ tx2.GetTransaction() };
    REQUIRE_NOTHROW(libmw::node::CheckTransaction(transaction));
    REQUIRE_THROWS(libmw::node::CheckTxInputs(pCachedView, transaction, height-1));
    REQUIRE_NOTHROW(libmw::node::CheckTxInputs(pCachedView, transaction, height));

    // Try to spend an unknown pegin output
    test::Tx tx3 = test::Tx::CreatePegIn2(1000, outputBF);
    const Output& output2 = tx3.GetOutputs().front();
    Input input2(output2.GetFeatures(), Commitment(output2.GetCommitment()));
    test::Tx tx4 = test::Tx::CreateSpend(input2, outputBF, 1000, 10);
    transaction = libmw::TxRef{ tx4.GetTransaction() };
    REQUIRE_NOTHROW(libmw::node::CheckTransaction(transaction));
    REQUIRE_THROWS(libmw::node::CheckTxInputs(pCachedView, transaction, height-1));
    REQUIRE_THROWS(libmw::node::CheckTxInputs(pCachedView, transaction, height));

    pNode.reset();
    LoggerAPI::Shutdown();
}