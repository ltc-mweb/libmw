#pragma once

#include <libmw/defs.h>
#include <libmw/interfaces/wallet_interface.h>
#include <mw/exceptions/InsufficientFundsException.h>

class CoinSelection
{
public:
    static std::vector<libmw::Coin> SelectCoins(
        const libmw::IWallet::Ptr& pWallet,
        const uint64_t amount)
    {
        std::vector<libmw::Coin> selected_coins = pWallet->SelectCoins(
            AvailableCoins(pWallet),
            amount
        );

        for (libmw::Coin& coin : selected_coins) {
            coin.spent = true;
            coin.spent_block.reset();
        }

        return selected_coins;
    }

private:
    static std::vector<libmw::Coin> AvailableCoins(const libmw::IWallet::Ptr& pWallet)
    {
        std::vector<libmw::Coin> coins = pWallet->ListCoins();
        std::vector<libmw::Coin> result;

        for (const libmw::Coin& coin : coins) {
            if (coin.spent || coin.spent_block.has_value()) {
                continue;
            }

            uint64_t num_confirmations = 0;
            if (coin.included_block.has_value()) {
                num_confirmations = pWallet->GetDepthInActiveChain(coin.included_block.value());
            }

            bool isPegin = coin.features & libmw::PEGIN_OUTPUT;
            bool peginMatured = num_confirmations >= mw::ChainParams::GetPegInMaturity();

            if (num_confirmations > 0 && (!isPegin || peginMatured)) {
                result.push_back(coin);
            }
        }

        return result;
    }
};