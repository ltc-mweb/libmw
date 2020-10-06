#pragma once

// Copyright (c) 2018-2020 David Burkett
// Copyright (c) 2020 The Litecoin Developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "defs.h"
#include "interfaces.h"
#include "interfaces/wallet.h"

#include <array>
#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include <unordered_map>

LIBMW_NAMESPACE

// TODO: Thoroughly document usage

MWIMPORT libmw::HeaderRef DeserializeHeader(const std::vector<uint8_t>& bytes);
MWIMPORT std::vector<uint8_t> SerializeHeader(const libmw::HeaderRef& header);

MWIMPORT libmw::BlockRef DeserializeBlock(const std::vector<uint8_t>& bytes);
MWIMPORT std::vector<uint8_t> SerializeBlock(const libmw::BlockRef& block);

MWIMPORT libmw::BlockUndoRef DeserializeBlockUndo(const std::vector<uint8_t>& bytes);
MWIMPORT std::vector<uint8_t> SerializeBlockUndo(const libmw::BlockUndoRef& blockUndo);

MWIMPORT libmw::TxRef DeserializeTx(const std::vector<uint8_t>& bytes);
MWIMPORT std::vector<uint8_t> SerializeTx(const libmw::TxRef& tx);

NODE_NAMESPACE

//
// Loads the state (MMRs mostly) into memory, and validates the current UTXO set.
// If successful, the CoinsViewDB will be returned which represents the state of the active chain.
// TODO: Document exceptions thrown.
//
MWIMPORT libmw::CoinsViewRef Initialize(
    const libmw::ChainParams& chainParams,
    const libmw::HeaderRef& header,
    const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper
);

//
// Validates the given state and replaces the existing state if valid.
// Use during initial sync.
//
MWIMPORT libmw::CoinsViewRef ApplyState(
    const libmw::IBlockStore::Ptr& pBlockStore,
    const libmw::IDBWrapper::Ptr& pCoinsDB,
    const libmw::BlockHash& firstMWHeaderHash,
    const libmw::BlockHash& stateHeaderHash,
    const libmw::StateRef& state
);

MWIMPORT void CheckBlock(
    const libmw::BlockRef& block,
    const std::vector<libmw::PegIn>& pegInCoins,
    const std::vector<libmw::PegOut>& pegOutCoins
);

MWIMPORT libmw::BlockUndoRef ConnectBlock(const libmw::BlockRef& block, const libmw::CoinsViewRef& view);
MWIMPORT void DisconnectBlock(const libmw::BlockUndoRef& undoData, const libmw::CoinsViewRef& view);
MWIMPORT libmw::BlockRef BuildNextBlock(
    const uint64_t height,
    const libmw::CoinsViewRef& view,
    const std::vector<libmw::TxRef>& transactions,
    const std::vector<libmw::PegIn>& pegInCoins,
    const std::vector<libmw::PegOut>& pegOutCoins
);
MWIMPORT void FlushCache(const libmw::CoinsViewRef& view, const std::unique_ptr<libmw::IDBBatch>& pBatch = nullptr);

// State
MWIMPORT libmw::StateRef DeserializeState(const std::vector<uint8_t>& bytes);
MWIMPORT std::vector<uint8_t> SerializeState(const libmw::StateRef& state);
MWIMPORT libmw::StateRef SnapshotState(const libmw::CoinsViewRef& view, const libmw::BlockHash& block_hash);

// Mempool
MWIMPORT void CheckTransaction(const libmw::TxRef& transaction);

END_NAMESPACE // node

WALLET_NAMESPACE

MWIMPORT std::pair<libmw::TxRef, libmw::PegIn> CreatePegInTx(
    const libmw::IWallet::Ptr& pWallet,
    const uint64_t amount
);

MWIMPORT std::pair<libmw::TxRef, libmw::PegOut> CreatePegOutTx(
    const libmw::IWallet::Ptr& pWallet,
    const uint64_t amount,
    const std::string& address
);

END_NAMESPACE // wallet
END_NAMESPACE // libmw