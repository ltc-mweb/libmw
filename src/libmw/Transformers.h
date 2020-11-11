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
        [](const libmw::PegOut& pegout) { return PegOutCoin{ pegout.amount, Bech32Address::FromString(pegout.address) }; }
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
    std::vector<Kernel> pegin_kernels = pBlock->GetPegInKernels();
    std::vector<libmw::PegIn> pegins;
    std::transform(
        pegin_kernels.cbegin(), pegin_kernels.cend(),
        std::back_inserter(pegins),
        [](const Kernel& kernel) {
            libmw::PegIn pegin;
            pegin.amount = kernel.GetAmount();
            const auto& commit = kernel.GetCommitment().vec();
            std::copy(commit.begin(), commit.end(), pegin.commitment.data());
            return pegin;
        }
    );

    std::vector<Kernel> pegout_kernels = pBlock->GetPegOutKernels();
    std::vector<libmw::PegOut> pegouts;
    std::transform(
        pegout_kernels.cbegin(), pegout_kernels.cend(),
        std::back_inserter(pegouts),
        [](const Kernel& kernel) {
            assert(kernel.GetAddress().has_value());

            libmw::PegOut pegout;
            pegout.amount = kernel.GetAmount();
            pegout.address = kernel.GetAddress().value().ToString();
            return pegout;
        }
    );

    return libmw::BlockAndPegs{ pBlock, pegins, pegouts };
}