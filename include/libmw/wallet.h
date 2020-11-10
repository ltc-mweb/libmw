#pragma once

#include "defs.h"
#include "interfaces/chain_interface.h"
#include "interfaces/wallet_interface.h"

LIBMW_NAMESPACE
WALLET_NAMESPACE

/// <summary>
/// Creates a peg-in transaction.
/// </summary>
/// <param name="pWallet">The wallet to create the transaction for. Must not be null.</param>
/// <param name="amount">The amount to peg-in.</param>
/// <returns>The non-null libmw::TxRef and libmw::PegIn that were created.</returns>
MWIMPORT std::pair<libmw::TxRef, libmw::PegIn> CreatePegInTx(
    const libmw::IWallet::Ptr& pWallet,
    const uint64_t amount
);

/// <summary>
/// Creates a peg-out transaction.
/// </summary>
/// <param name="pWallet">The wallet to create the transaction for. Must not be null.</param>
/// <param name="amount">The amount to pegout. Must be positive.</param>
/// <param name="fee_base">The base fee rate to be multiplied by transaction weight.</param>
/// <param name="address">The LTC address to send the coins to. Must be a valid bech32 address.</param>
/// <returns>The non-null libmw::TxRef and libmw::PegOut that were created.</returns>
MWIMPORT std::pair<libmw::TxRef, libmw::PegOut> CreatePegOutTx(
    const libmw::IWallet::Ptr& pWallet,
    const uint64_t amount,
    const uint64_t fee_base,
    const std::string& address
);

/// <summary>
/// Initiates a send to the MWEB address specified.
/// </summary>
/// <param name="pWallet">The wallet to send from. Must not be null.</param>
/// <param name="amount">The amount to send. Must be positive.</param>
/// <param name="fee_base">The base fee rate to be multiplied by transaction weight.</param>
/// <param name="address">The address to send to.</param>
/// <returns>A partial transaction to be finalized by the receiver.</returns>
MWIMPORT libmw::PartialTransaction Send(
    const libmw::IWallet::Ptr& pWallet,
    const uint64_t amount,
    const uint64_t fee_base,
    const libmw::MWEBAddress& address
);

/// <summary>
/// Receives coins by finishing a PartialTransaction.
/// </summary>
/// <param name="pWallet">The wallet to receive to. Must not be null.</param>
/// <param name="partialTx">The partial transaction to finish.</param>
/// <returns>The finished transaction, ready to be broadcast.</returns>
MWIMPORT libmw::TxRef Receive(
    const libmw::IWallet::Ptr& pWallet,
    const libmw::PartialTransaction& partialTx
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

MWIMPORT void ScanForOutputs(
    const libmw::IWallet::Ptr& pWallet,
    const libmw::IChain::Ptr& pChain
);

/// <summary>
/// Calculates the MWEB wallet address.
/// Currently, this always generates an address using a pre-defined bip32 keychain path.
/// TODO: Add multi-address support.
/// </summary>
/// <param name="pWallet">The wallet to calculate the MWEB wallet address for. Must not be null.</param>
/// <returns></returns>
MWIMPORT MWEBAddress GetAddress(const libmw::IWallet::Ptr& pWallet);

/// <summary>
/// Calculates the balances of the wallet.
/// </summary>
/// <param name="pWallet">The wallet to update. Must not be null.</param>
/// <returns>The confirmed, unconfirmed, and immature balances.</returns>
MWIMPORT WalletBalance GetBalance(const libmw::IWallet::Ptr& pWallet);

END_NAMESPACE // wallet
END_NAMESPACE // libmw