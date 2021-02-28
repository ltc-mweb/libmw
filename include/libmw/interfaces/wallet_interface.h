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
    /// Retrieves the highest known subaddress key index used by the wallet.
    /// </summary>
    /// <returns>The highest index of a subaddress known to be generated for the wallet.</returns>
    //virtual uint32_t GetHighestIndex() = 0;

    /// <summary>
    /// Calculates the private key at the given bip32 path.
    /// The key is NOT written to the database.
    /// </summary>
    /// <param name="bip32Path">eg. "m/44'/0/1/2"</param>
    /// <returns>The bip32-generated libmw::PrivateKey.</returns>
    virtual libmw::PrivateKey GetHDKey(const std::string& bip32Path) const = 0;

    /// <summary>
    /// Retrieves the MWEB coin with the matching commitment.
    /// </summary>
    /// <returns>True if the coin was found.</returns>
    virtual bool GetCoin(const libmw::Commitment& output_commit, libmw::Coin& coin) const = 0;
};

END_NAMESPACE