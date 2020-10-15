#pragma once

#include "defs.h"
#include <libmw/interfaces/db_interface.h>

// TODO: Document usage
LIBMW_NAMESPACE
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
MWIMPORT libmw::StateRef SnapshotState(const libmw::CoinsViewRef& view, const libmw::BlockHash& block_hash);

// Mempool
MWIMPORT void CheckTransaction(const libmw::TxRef& transaction);

END_NAMESPACE // node
END_NAMESPACE // libmw