#include <catch.hpp>

#include <test_framework/TestWallet.h>
#include <mw/models/block/Block.h>
#include <mw/models/tx/Transaction.h>
#include <mw/config/ChainParams.h>
#include <libmw/libmw.h>

TEST_CASE("Wallet - Peg-ins")
{
    TestWallet::Ptr pTestWallet = TestWallet::Create();

    // Peg-in 5,000,000 litoshis
    auto pegin_tx = libmw::wallet::CreatePegInTx(pTestWallet, 5'000'000);
    libmw::node::CheckTransaction(pegin_tx.first);

    // Balance should have 5,000,000 unconfirmed litoshis
    libmw::WalletBalance balance = libmw::wallet::GetBalance(pTestWallet);
    REQUIRE(balance.confirmed_balance == 0);
    REQUIRE(balance.unconfirmed_balance == 5'000'000);
    REQUIRE(balance.immature_balance == 0);
    REQUIRE(balance.locked_balance == 0);

    // Include peg-in tx in block1 with depth: PEGIN_MATURITY - 1
    auto pBlock = std::make_shared<mw::Block>(
        std::make_shared<mw::Header>(),
        pegin_tx.first.pTransaction->GetBody()
    );
    auto block1_hash = Random::CSPRNG<32>().GetBigInt().ToArray();
    libmw::wallet::BlockConnected(pTestWallet, libmw::BlockRef{ pBlock }, block1_hash);
    pTestWallet->SetDepthInActiveChain(block1_hash, mw::ChainParams::GetPegInMaturity() - 1);

    // Balance should have 5,000,000 immature litoshis
    balance = libmw::wallet::GetBalance(pTestWallet);
    REQUIRE(balance.confirmed_balance == 0);
    REQUIRE(balance.unconfirmed_balance == 0);
    REQUIRE(balance.immature_balance == 5'000'000);
    REQUIRE(balance.locked_balance == 0);

    // Set block1 depth: PEGIN_MATURITY
    pTestWallet->SetDepthInActiveChain(block1_hash, mw::ChainParams::GetPegInMaturity());

    // Balance should have 5,000,000 confirmed litoshis
    balance = libmw::wallet::GetBalance(pTestWallet);
    REQUIRE(balance.confirmed_balance == 5'000'000);
    REQUIRE(balance.unconfirmed_balance == 0);
    REQUIRE(balance.immature_balance == 0);
    REQUIRE(balance.locked_balance == 0);
}