#pragma once

#include <libmw/defs.h>
#include <mw/models/crypto/BlindingFactor.h>
#include <mw/crypto/Crypto.h>

class WalletUtil
{
public:
    static BlindingFactor AddBlindingFactors(const std::vector<libmw::Coin>& coins)
    {
        std::vector<BlindingFactor> blinds;

        std::transform(
            coins.cbegin(), coins.cend(),
            std::back_inserter(blinds),
            [](const libmw::Coin& coin) {
                assert(coin.key.has_value());
                return BlindingFactor(coin.key.value().keyBytes);
            }
        );

        return Crypto::AddBlindingFactors(blinds);
    }
    
    static uint64_t TotalAmount(const std::vector<libmw::Coin>& coins)
    {
        return std::accumulate(
            coins.cbegin(), coins.cend(), (uint64_t)0,
            [](uint64_t total, const libmw::Coin& coin) { return total + coin.amount; }
        );
    }

    // TODO: Determine fee algo
    static uint64_t CalculateFee(
        const uint64_t fee_base,
        const uint64_t num_inputs,
        const uint64_t num_kernels,
        const uint64_t num_outputs)
    {
        return fee_base * (num_inputs + (num_kernels * 4) + (num_outputs * 8));
    }
};