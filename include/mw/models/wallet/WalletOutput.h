#pragma once

#include <mw/models/tx/Output.h>
#include <mw/models/crypto/SecretKey.h>
#include <mw/db/IBlockStore.h>
#include <boost/optional.hpp>

struct WalletOutput
{
    enum class Status
    {
        CONFIRMED,
        UNCONFIRMED,
        SPENT,
        LOCKED,
        CANCELED
    };

    Output output;
    SecretKey key;
    uint64_t amount;
    Status status;
    boost::optional<mw::Hash> block_hash;

    //
    // Checks if the output is on-chain, and has at least the given number of confirmations.
    //
    bool IsSpendable(
        const mw::IBlockStore& block_store,
        const uint64_t current_block_height,
        const uint64_t minimum_confirmations) const noexcept
    {
        if (status == Status::CONFIRMED && block_hash.has_value()) {
            mw::Header::CPtr pHeader = block_store.GetHeader(block_hash.value());
            if (pHeader != nullptr && current_block_height >= pHeader->GetHeight()) {
                return (current_block_height + 1) - pHeader->GetHeight() >= minimum_confirmations;
            }
        }

        return false;
    }
};