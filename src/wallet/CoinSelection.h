#pragma once

#include <libmw/defs.h>
#include <mw/exceptions/InsufficientFundsException.h>

class CoinSelection
{
public:
    static std::vector<libmw::Coin> SelectCoins(
        const std::vector<libmw::Coin>& coins,
        const uint64_t amount,
        const uint64_t fee_base)
    {
        std::vector<libmw::Coin> selected_coins;
        
        uint64_t fee = 10 * fee_base;// TODO: Implement actual fee calculation
        uint64_t inputs_amount = 0;
        for (const libmw::Coin& coin : coins) {
            if (coin.spent) {
                continue;
            }

            inputs_amount += coin.amount;
            selected_coins.push_back(coin);
            if (inputs_amount >= amount + fee) {
                break;
            }
        }

        if (inputs_amount <= amount + fee) {
            ThrowInsufficientFunds("Not enough funds");
        }

        return selected_coins;
    }
};