#pragma once

#include <ext/libmw.h>
#include <mw/models/tx/PegInCoin.h>
#include <mw/models/tx/PegOutCoin.h>

static std::vector<PegInCoin> TransformPegIns(const std::vector<mw::PegIn>& pegInCoins)
{
    std::vector<PegInCoin> pegins;
    std::transform(
        pegInCoins.cbegin(), pegInCoins.cend(),
        std::back_inserter(pegins),
        [](const mw::PegIn& pegin) { return PegInCoin{ pegin.amount, Commitment{ pegin.commitment } }; }
    );

    return pegins;
}

static std::vector<PegOutCoin> TransformPegOuts(const std::vector<mw::PegOut>& pegOutCoins)
{
    std::vector<PegOutCoin> pegouts;
    std::transform(
        pegOutCoins.cbegin(), pegOutCoins.cend(),
        std::back_inserter(pegouts),
        [](const mw::PegOut& pegout) { return PegOutCoin{ pegout.amount, Bech32Address::FromString(pegout.address) }; }
    );

    return pegouts;
}