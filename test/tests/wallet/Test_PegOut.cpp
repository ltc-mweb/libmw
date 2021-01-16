#include <catch.hpp>

#include <test_framework/TestWallet.h>
#include <mw/consensus/Weight.h>
#include <mw/models/block/Block.h>
#include <mw/models/tx/Transaction.h>
#include <mw/config/ChainParams.h>
#include <libmw/libmw.h>

TEST_CASE("Wallet - Peg-out")
{
    TestWallet::Ptr pTestWallet = TestWallet::Create();

    // Peg-in 5,000,000 litoshis
    auto pegin_tx = libmw::wallet::CreatePegInTx(pTestWallet, 5'000'000);

    // Include peg-in tx in block1 with depth: PEGIN_MATURITY
    auto pBlock1 = std::make_shared<mw::Block>(
        std::make_shared<mw::Header>(),
        pegin_tx.first.pTransaction->GetBody()
    );
    auto block1_hash = pBlock1->GetHash().ToArray();
    libmw::wallet::BlockConnected(pTestWallet, libmw::BlockRef{ pBlock1 }, block1_hash);
    pTestWallet->SetDepthInActiveChain(block1_hash, mw::ChainParams::GetPegInMaturity());

    // Balance should have 5,000,000 confirmed litoshis
    libmw::WalletBalance balance = libmw::wallet::GetBalance(pTestWallet);
    REQUIRE(balance.confirmed_balance == 5'000'000);
    REQUIRE(balance.unconfirmed_balance == 0);
    REQUIRE(balance.immature_balance == 0);
    REQUIRE(balance.locked_balance == 0);

    // Peg-out 3,000,000 litoshis
    auto pegout_tx = libmw::wallet::CreatePegOutTx(pTestWallet, 3'000'000, 10'000, "tltc1qh50sy0823vxn4l9zk2820w4fuj0q4fgjza5vv");
    auto pegouts = pegout_tx.first.GetPegouts();
    REQUIRE(pegouts.size() == 1);
    REQUIRE(pegouts[0].address == "tltc1qh50sy0823vxn4l9zk2820w4fuj0q4fgjza5vv");
    REQUIRE(pegouts[0].amount == 3'000'000);

    uint64_t expected_fee = 10'000 * Weight::Calculate({ .num_kernels = 1, .num_owner_sigs = 1, .num_outputs = 2 });
    uint64_t change_amount = (2'000'000 - expected_fee);
    REQUIRE(pegout_tx.first.pTransaction->GetPegOutKernels().size() == 1);
    REQUIRE(pegout_tx.first.pTransaction->GetPegOutKernels()[0].GetFee() == expected_fee);

    libmw::node::CheckTransaction(pegout_tx.first);

    // Balance should have (2'000'000 - fee) unconfirmed litoshis
    balance = libmw::wallet::GetBalance(pTestWallet);
    REQUIRE(balance.confirmed_balance == 0);
    REQUIRE(balance.unconfirmed_balance == change_amount);
    REQUIRE(balance.immature_balance == 0);
    REQUIRE(balance.locked_balance == 5'000'000);

    // Include peg-out tx in block2
    auto pBlock2 = std::make_shared<mw::Block>(
        std::make_shared<mw::Header>(),
        pegout_tx.first.pTransaction->GetBody()
    );
    auto block2_hash = pBlock2->GetHash().ToArray();
    libmw::wallet::BlockConnected(pTestWallet, libmw::BlockRef{ pBlock2 }, block2_hash);
    pTestWallet->SetDepthInActiveChain(block2_hash, 1);

    // Balance should have 2,000,000 confirmed litoshis
    balance = libmw::wallet::GetBalance(pTestWallet);
    REQUIRE(balance.confirmed_balance == change_amount);
    REQUIRE(balance.unconfirmed_balance == 0);
    REQUIRE(balance.immature_balance == 0);
    REQUIRE(balance.locked_balance == 0);
}