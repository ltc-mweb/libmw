#include <catch.hpp>

#include <mw/node/validation/BlockValidator.h>
#include <test_framework/Node.h>
#include <test_framework/Miner.h>

TEST_CASE("BlockValidator")
{
    test::Miner miner;

    // Block 10
    std::vector<test::Tx> block_10_txs{
        test::Tx::CreatePegIn(5'000'000)
    };

    std::vector<PegInCoin> pegInCoins{
        PegInCoin(5'000'000, block_10_txs[0].GetTransaction()->GetKernels()[0].GetCommitment())
    };
    std::vector<PegOutCoin> pegOutCoins;
    test::MinedBlock block_10 = miner.MineBlock(10, block_10_txs);

    BlockValidator().Validate(block_10.GetBlock(), pegInCoins, pegOutCoins);
}