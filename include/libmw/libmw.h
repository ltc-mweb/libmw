#pragma once

// Copyright (c) 2018-2020 David Burkett
// Copyright (c) 2020 The Litecoin Developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "defs.h"
#include "interfaces.h"

#include <array>
#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include <unordered_map>

LIBMW_NAMESPACE

// TODO: Thoroughly document usage

IMPORT libmw::HeaderRef DeserializeHeader(std::vector<uint8_t>&& bytes);
IMPORT std::vector<uint8_t> SerializeHeader(const libmw::HeaderRef& header);

IMPORT libmw::BlockRef DeserializeBlock(std::vector<uint8_t>&& bytes);
IMPORT std::vector<uint8_t> SerializeBlock(const libmw::BlockRef& block);

IMPORT libmw::BlockUndoRef DeserializeBlockUndo(std::vector<uint8_t>&& bytes);
IMPORT std::vector<uint8_t> SerializeBlockUndo(const libmw::BlockUndoRef& blockUndo);

IMPORT libmw::TxRef DeserializeTx(std::vector<uint8_t>&& bytes);
IMPORT std::vector<uint8_t> SerializeTx(const libmw::TxRef& tx);

NODE_NAMESPACE

//
// Loads the state (MMRs mostly) into memory, and validates the current UTXO set.
// If successful, the CoinsViewDB will be returned which represents the state of the active chain.
// TODO: Document exceptions thrown.
//
IMPORT libmw::CoinsViewRef Initialize(
    const libmw::ChainParams& chainParams,
    const libmw::HeaderRef& header,
    const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper
);

//
// Validates the given state and replaces the existing state if valid.
// Use during initial sync.
//
IMPORT libmw::CoinsViewRef ApplyState(
    const libmw::IBlockStore::Ptr& pBlockStore,
    const libmw::IDBWrapper::Ptr& pCoinsDB,
    const libmw::BlockHash& firstMWHeaderHash,
    const libmw::BlockHash& stateHeaderHash,
    const libmw::StateRef& state
);

IMPORT void CheckBlock(
    const libmw::BlockRef& block,
    const std::vector<libmw::PegIn>& pegInCoins,
    const std::vector<libmw::PegOut>& pegOutCoins
);

IMPORT libmw::BlockUndoRef ConnectBlock(const libmw::BlockRef& block, const libmw::CoinsViewRef& view);
IMPORT void DisconnectBlock(const libmw::BlockUndoRef& undoData, const libmw::CoinsViewRef& view);
IMPORT libmw::BlockRef BuildNextBlock(
    const uint64_t height,
    const libmw::CoinsViewRef& view,
    const std::vector<libmw::TxRef>& transactions,
    const std::vector<libmw::PegIn>& pegInCoins,
    const std::vector<libmw::PegOut>& pegOutCoins
);
IMPORT void FlushCache(const libmw::CoinsViewRef& view, const std::unique_ptr<libmw::IDBBatch>& pBatch = nullptr);

// State
IMPORT libmw::StateRef DeserializeState(std::vector<uint8_t>&& bytes);
IMPORT std::vector<uint8_t> SerializeState(const libmw::StateRef& state);
IMPORT libmw::StateRef SnapshotState(const libmw::CoinsViewRef& view, const libmw::BlockHash& block_hash);

// Mempool
IMPORT void CheckTransaction(const libmw::TxRef& transaction);

END_NAMESPACE // node
END_NAMESPACE // libmw