#pragma once

#include <libmw/libmw.h>
#include <mw/models/tx/PegInCoin.h>
#include <mw/models/tx/PegOutCoin.h>
#include <mw/models/tx/Transaction.h>
#include <mw/models/block/Block.h>

static mw::Hash TransformHash(const libmw::BlockHash& hash)
{
    return mw::Hash(hash);
}

static std::vector<PegInCoin> TransformPegIns(const std::vector<libmw::PegIn>& pegInCoins)
{
    std::vector<PegInCoin> pegins;
    std::transform(
        pegInCoins.cbegin(), pegInCoins.cend(),
        std::back_inserter(pegins),
        [](const libmw::PegIn& pegin) { return PegInCoin{ pegin.amount, Commitment{ pegin.commitment } }; }
    );

    return pegins;
}

static std::vector<PegOutCoin> TransformPegOuts(const std::vector<libmw::PegOut>& pegOutCoins)
{
    std::vector<PegOutCoin> pegouts;
    std::transform(
        pegOutCoins.cbegin(), pegOutCoins.cend(),
        std::back_inserter(pegouts),
        [](const libmw::PegOut& pegout) { return PegOutCoin{ pegout.amount, pegout.scriptPubKey }; }
    );

    return pegouts;
}

static std::vector<mw::Transaction::CPtr> TransformTxs(const std::vector<libmw::TxRef>& txs)
{
    std::vector<mw::Transaction::CPtr> transactions;
    std::transform(
        txs.cbegin(), txs.cend(),
        std::back_inserter(transactions),
        [](const libmw::TxRef& tx) { return tx.pTransaction; }
    );

    return transactions;
}

static libmw::BlockAndPegs TransformBlock(const mw::Block::Ptr& pBlock)
{
    std::vector<PegInCoin> pegin_coins = pBlock->GetPegIns();
    std::vector<libmw::PegIn> pegins;
    std::transform(
        pegin_coins.cbegin(), pegin_coins.cend(),
        std::back_inserter(pegins),
        [](const PegInCoin& pegin) {
            return libmw::PegIn{ pegin.GetAmount(), pegin.GetCommitment().array() };
        }
    );

    std::vector<PegOutCoin> pegout_coins = pBlock->GetPegOuts();
    std::vector<libmw::PegOut> pegouts;
    std::transform(
        pegout_coins.cbegin(), pegout_coins.cend(),
        std::back_inserter(pegouts),
        [](const PegOutCoin& pegout) {
            return libmw::PegOut{ pegout.GetAmount(), pegout.GetScriptPubKey() };
        }
    );

    return libmw::BlockAndPegs{ pBlock, pegins, pegouts };
}

static std::vector<Commitment> TransformCommitments(const std::vector<libmw::Commitment>& libmw_commits)
{
    std::vector<Commitment> commits;
    std::transform(
        libmw_commits.cbegin(), libmw_commits.cend(),
        std::back_inserter(commits),
        [](const libmw::Commitment& commit) { return Commitment(commit); }
    );

    return commits;
}