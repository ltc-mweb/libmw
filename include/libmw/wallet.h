#pragma once

#include "defs.h"
#include "interfaces/chain_interface.h"
#include "interfaces/wallet_interface.h"
#include <boost/variant.hpp>

LIBMW_NAMESPACE

struct PegOutRecipient
{
    uint64_t amount;
    std::string address;
};

struct MWEBRecipient
{
    uint64_t amount;
    libmw::MWEBAddress address;
};

struct PegInRecipient
{
    uint64_t amount;
    libmw::MWEBAddress address;
};

typedef boost::variant<MWEBRecipient, PegInRecipient, PegOutRecipient> Recipient;

WALLET_NAMESPACE

MWIMPORT libmw::TxRef CreateTx(
    const libmw::IWallet::Ptr& pWallet,
    const std::vector<libmw::Commitment>& selected_inputs,
    const std::vector<libmw::Recipient>& recipients,
    const boost::optional<uint64_t>& pegin_amount,
    const uint64_t fee
);

/// <summary>
/// Checks the transactions in the block to see if any belong to the wallet, updating the wallet accordingly.
/// This should be called when connecting a block to the active chain.
/// </summary>
/// <param name="pWallet">The wallet to update. Must not be null.</param>
/// <param name="block">The connected MW extension block. Must not be null.</param>
/// <param name="canonical_block_hash">The hash of the canonical block containing the extension block.</param>
MWIMPORT void BlockConnected(
    const libmw::IWallet::Ptr& pWallet,
    const libmw::BlockRef& block,
    const libmw::BlockHash& canonical_block_hash
);

/// <summary>
/// Checks the wallet for any transactions from the block, and updates their status accordingly.
/// This should be called when disconnecting a block from the active chain.
/// </summary>
/// <param name="pWallet">The wallet to update. Must not be null.</param>
/// <param name="block">The disconnected MW extension block. Must not be null.</param>
MWIMPORT void BlockDisconnected(
    const libmw::IWallet::Ptr& pWallet,
    const libmw::BlockRef& block
);

/// <summary>
/// Checks the transaction to see if it belongs to the wallet, updating the wallet accordingly.
/// This should be called when the transaction is added to the mempool.
/// </summary>
/// <param name="pWallet">The wallet to update. Must not be null.</param>
/// <param name="tx">The transaction that was added to the mempool. Must not be null.</param>
MWIMPORT void TransactionAddedToMempool(
    const libmw::IWallet::Ptr& pWallet,
    const libmw::TxRef& tx
);

MWIMPORT void ScanForOutputs(
    const libmw::IWallet::Ptr& pWallet,
    const libmw::IChain::Ptr& pChain
);

/// <summary>
/// Computes the MWEB wallet address.
/// Currently, this always generates an address using a pre-defined bip32 keychain path.
/// FUTURE: Add multi-address support.
/// </summary>
/// <param name="pWallet">The wallet to calculate the MWEB wallet address for. Must not be null.</param>
/// <param name="index">The index of the address keypair to use.</param>
/// <returns>The MWEB wallet's bech32 address.</returns>
MWIMPORT MWEBAddress GetAddress(const libmw::IWallet::Ptr& pWallet, const uint32_t index);

/// <summary>
/// Determines if the MWEB wallet address belongs to the given wallet.
/// </summary>
/// <param name="pWallet">The wallet to check. Must not be null.</param>
/// <param name="address">The bech32 address to check ownership of.</param>
/// <returns>True if the address belongs to the wallet.</returns>
MWIMPORT bool IsOwnAddress(const libmw::IWallet::Ptr& pWallet, const MWEBAddress& address);

/// <summary>
/// Calculates the balances of the wallet.
/// </summary>
/// <param name="pWallet">The wallet to update. Must not be null.</param>
/// <returns>The confirmed, unconfirmed, and immature balances.</returns>
MWIMPORT WalletBalance GetBalance(const libmw::IWallet::Ptr& pWallet);

MWIMPORT bool RewindOutput(
    const libmw::IWallet::Ptr& pWallet,
    const libmw::TxRef& tx,
    const libmw::Commitment& output_commit,
    libmw::Coin& coin_out
);

END_NAMESPACE // wallet
END_NAMESPACE // libmw