#pragma once

#include "defs.h"
#include "interfaces/chain_interface.h"
#include "interfaces/wallet_interface.h"
#include <boost/variant.hpp>

LIBMW_NAMESPACE

struct PegOutRecipient
{
    uint64_t amount;

    /// <summary>
    /// 4-42 bytes
    /// </summary>
    std::vector<uint8_t> scriptPubKey;
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
/// Computes the MWEB wallet address.
/// Currently, this always generates an address using a pre-defined bip32 keychain path.
/// FUTURE: Add multi-address support.
/// </summary>
/// <param name="pWallet">The wallet to calculate the MWEB wallet address for. Must not be null.</param>
/// <param name="index">The index of the address keypair to use.</param>
/// <returns>The MWEB wallet's bech32 address.</returns>
MWIMPORT MWEBAddress GetAddress(const libmw::IWallet::Ptr& pWallet, const uint32_t index);

MWIMPORT bool RewindOutput(
    const libmw::IWallet::Ptr& pWallet,
    const boost::variant<libmw::TxRef, libmw::BlockRef>& parent,
    const libmw::Commitment& output_commit,
    libmw::Coin& coin_out
);

END_NAMESPACE // wallet
END_NAMESPACE // libmw