#include <catch.hpp>

#include <test_framework/TestWallet.h>
#include <mw/models/block/Block.h>
#include <mw/models/tx/Transaction.h>
#include <mw/config/ChainParams.h>
#include <libmw/libmw.h>

TEST_CASE("Wallet - SendReceive")
{
    TestWallet::Ptr pSendWallet = TestWallet::Create();
    TestWallet::Ptr pRecvWallet = TestWallet::Create();

    // Peg-in 5,000,000 litoshis
    auto pegin_tx = libmw::wallet::CreatePegInTx(pSendWallet, 5'000'000);

    // Balance should have 5,000,000 unconfirmed litoshis
    libmw::WalletBalance balance = libmw::wallet::GetBalance(pSendWallet);
    REQUIRE(balance.confirmed_balance == 0);
    REQUIRE(balance.unconfirmed_balance == 5'000'000);
    REQUIRE(balance.immature_balance == 0);
    REQUIRE(balance.locked_balance == 0);

    // Include peg-in tx in block1 with depth: PEGIN_MATURITY
    auto pBlock1 = std::make_shared<mw::Block>(
        std::make_shared<mw::Header>(),
        pegin_tx.first.pTransaction->GetBody()
    );
    auto block1_hash = Random::CSPRNG<32>().GetBigInt().ToArray();
    libmw::wallet::BlockConnected(pSendWallet, libmw::BlockRef{ pBlock1 }, block1_hash);
    pSendWallet->SetDepthInActiveChain(block1_hash, mw::ChainParams::GetPegInMaturity());

    // Balance should have 5,000,000 confirmed litoshis
    balance = libmw::wallet::GetBalance(pSendWallet);
    REQUIRE(balance.confirmed_balance == 5'000'000);
    REQUIRE(balance.unconfirmed_balance == 0);
    REQUIRE(balance.immature_balance == 0);
    REQUIRE(balance.locked_balance == 0);

    // Initiate 2,000,000 litoshi spend
    libmw::MWEBAddress recv_address = libmw::wallet::GetAddress(pRecvWallet, 0);
    libmw::TxRef received_tx = libmw::wallet::Send(pSendWallet, 2'000'000, 0, recv_address);
    libmw::node::CheckTransaction(received_tx);

    // Balance should have 5'000'000 locked litoshis and 3'000'000 unconfirmed
    balance = libmw::wallet::GetBalance(pSendWallet);
    REQUIRE(balance.confirmed_balance == 0);
    REQUIRE(balance.unconfirmed_balance == 3'000'000);
    REQUIRE(balance.immature_balance == 0);
    REQUIRE(balance.locked_balance == 5'000'000);

    //// Receiver balance should have 2'000'000 unconfirmed litoshis
    //balance = libmw::wallet::GetBalance(pRecvWallet);
    //REQUIRE(balance.confirmed_balance == 0);
    //REQUIRE(balance.unconfirmed_balance == 2'000'000);
    //REQUIRE(balance.immature_balance == 0);
    //REQUIRE(balance.locked_balance == 0);

    // Include transaction in a block
    auto pBlock2 = std::make_shared<mw::Block>(
        std::make_shared<mw::Header>(),
        received_tx.pTransaction->GetBody()
    );
    auto block2_hash = Random::CSPRNG<32>().GetBigInt().ToArray();

    libmw::wallet::BlockConnected(pSendWallet, libmw::BlockRef{ pBlock2 }, block2_hash);
    pSendWallet->SetDepthInActiveChain(block2_hash, 1);

    libmw::wallet::BlockConnected(pRecvWallet, libmw::BlockRef{ pBlock2 }, block2_hash);
    pRecvWallet->SetDepthInActiveChain(block2_hash, 1);

    // Sender balance should have 3'000'000 confirmed litoshis
    balance = libmw::wallet::GetBalance(pSendWallet);
    REQUIRE(balance.confirmed_balance == 3'000'000);
    REQUIRE(balance.unconfirmed_balance == 0);
    REQUIRE(balance.immature_balance == 0);
    REQUIRE(balance.locked_balance == 0);

    // Receiver balance should have 2'000'000 confirmed litoshis
    balance = libmw::wallet::GetBalance(pRecvWallet);
    REQUIRE(balance.confirmed_balance == 2'000'000);
    REQUIRE(balance.unconfirmed_balance == 0);
    REQUIRE(balance.immature_balance == 0);
    REQUIRE(balance.locked_balance == 0);
}