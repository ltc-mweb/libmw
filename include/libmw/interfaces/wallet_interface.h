#pragma once

// Copyright (c) 2018-2020 David Burkett
// Copyright (c) 2020 The Litecoin Developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <libmw/defs.h>

LIBMW_NAMESPACE

/// <summary>
/// This interface provides methods for interacting with the core wallet.
/// Consumers of libmw::wallet functions are expected to provide an implementation of this.
/// </summary>
class IWallet
{
public:
    using Ptr = std::shared_ptr<libmw::IWallet>;

    virtual ~IWallet() = default;

    /// <summary>
    /// Generates a new HD key and saves it to the wallet database.
    /// </summary>
    /// <returns>The newly-generated libmw::PrivateKey.</returns>
    virtual libmw::PrivateKey GenerateNewHDKey() = 0;

    /// <summary>
    /// Calculates the private key at the given bip32 path.
    /// The key is NOT written to the database.
    /// </summary>
    /// <param name="bip32Path">eg. "m/44'/0/1/2"</param>
    /// <returns>The bip32-generated libmw::PrivateKey.</returns>
    virtual libmw::PrivateKey GetHDKey(const std::string& bip32Path) const = 0;

    /// <summary>
    /// Retrieves all MWEB coins from the wallet's database.
    /// </summary>
    /// <returns>A possibly-empty vector of the wallet's MWEB coins.</returns>
    virtual std::vector<libmw::Coin> ListCoins() const = 0;

    /// <summary>
    /// Adds the MWEB coins to the wallet's database.
    /// </summary>
    /// <param name="coins">A possibly-empty vector of MWEB coins to add to the wallet's DB.</param>
    virtual void AddCoins(const std::vector<libmw::Coin>& coins) = 0;

    /// <summary>
    /// Removes the MWEB coins from the wallet's database.
    /// </summary>
    /// <param name="coins">A possibly-empty vector of MWEB coins to remove from the wallet's DB.</param>
    virtual void DeleteCoins(const std::vector<libmw::Coin>& coins) = 0;

    /// <summary>
    /// Returns the depth of the block in the active chain.
    /// </summary>
    /// <param name="canonical_block_hash">The *canonical* block hash.</param>
    /// <returns>
    /// The depth in the active chain.
    /// 1 for chain tip, 2 for previous block, etc.
    /// 0 if not in the chain.
    /// </returns>
    virtual uint64_t GetDepthInActiveChain(const libmw::BlockHash& canonical_block_hash) = 0;
};

END_NAMESPACE