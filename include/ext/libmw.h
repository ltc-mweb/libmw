#pragma once

// Copyright (c) 2018-2020 David Burkett
// Copyright (c) 2020 The Litecoin Developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "interfaces.h"

#include <array>
#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include <unordered_map>

#if defined(_MSC_VER)
//  Microsoft 
#define EXPORT __declspec(dllexport)
#define IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
//  GCC
#define EXPORT __attribute__((visibility("default")))
#define IMPORT 
#else
#define EXPORT
#define IMPORT
#endif

#define MW_NAMESPACE namespace mw {
#define NODE_NAMESPACE namespace node {
#define DB_NAMESPACE namespace db {
#define END_NAMESPACE }

class CDBWrapper;

MW_NAMESPACE

class Header;
class Block;
class Transaction;
class ICoinsView;
struct State;

typedef std::array<uint8_t, 32> BlockHash;

struct PegIn
{
    uint64_t amount;
    std::array<uint8_t, 33> commitment;
};

struct PegOut
{
    uint64_t amount;
    std::string address;
};

struct HeaderRef
{
    std::shared_ptr<const mw::Header> pHeader;
};

struct HeaderAndPegsRef
{
    HeaderRef header;
    std::vector<PegIn> pegins;
    std::vector<PegOut> pegouts;
};

struct BlockRef
{
    std::shared_ptr<mw::Block> pBlock;
};

struct TxRef
{
    IMPORT uint64_t GetTotalFee() const noexcept;

    std::shared_ptr<mw::Transaction> pTransaction;
};

struct CoinsViewRef
{
    //
    // Creates a new CoinsViewCache on top of this CoinsView.
    //
    IMPORT CoinsViewRef CreateCache() const;

    std::shared_ptr<mw::ICoinsView> pCoinsView;
};

struct StateRef
{
    std::shared_ptr<mw::State> pState;
};

DB_NAMESPACE

//
// Interface for looking up blocks and headers.
// This must be implemented by the libmw consumer.
//
class IBlockStore
{
public:
    virtual ~IBlockStore() = default;

    virtual mw::HeaderRef GetHeader(const uint64_t height) const /*throw(NotFoundException)*/ = 0;
    virtual mw::HeaderRef GetHeader(const mw::BlockHash& hash) const /*throw(NotFoundException)*/ = 0;

    virtual mw::HeaderAndPegsRef GetHeaderAndPegs(const uint64_t height) const /*throw(NotFoundException)*/ = 0;
    virtual mw::HeaderAndPegsRef GetHeaderAndPegs(const mw::BlockHash& hash) const /*throw(NotFoundException)*/ = 0;

    virtual mw::BlockRef GetBlock(const uint64_t height) const /*throw(NotFoundException)*/ = 0;
    virtual mw::BlockRef GetBlock(const mw::BlockHash& hash) const /*throw(NotFoundException)*/ = 0;
};

END_NAMESPACE

// TODO: Thoroughly document usage

IMPORT mw::HeaderRef DeserializeHeader(std::vector<uint8_t>&& bytes);
IMPORT std::vector<uint8_t> SerializeHeader(const mw::HeaderRef& header);

IMPORT mw::BlockRef DeserializeBlock(std::vector<uint8_t>&& bytes);
IMPORT std::vector<uint8_t> SerializeBlock(const mw::BlockRef& block);

IMPORT mw::TxRef DeserializeTx(std::vector<uint8_t>&& bytes);
IMPORT std::vector<uint8_t> SerializeTx(const mw::TxRef& tx);

NODE_NAMESPACE

//
// Loads the state (MMRs mostly) into memory, and validates the current UTXO set.
// If successful, the CoinsViewDB will be returned which represents the state of the active chain.
// TODO: Document exceptions thrown.
//
IMPORT mw::CoinsViewRef Initialize(
    const std::string& datadir,
    const std::shared_ptr<mw::db::IDBWrapper>& pDBWrapper
);

//
// Validates the given state and replaces the existing state if valid.
//
IMPORT mw::CoinsViewRef ApplyState(
    const std::string& datadir,
    const mw::db::IBlockStore* pBlockStore,
    const mw::BlockHash& firstMWHeaderHash,
    const mw::BlockHash& stateHeaderHash,
    CDBWrapper* pDatabase,
    const mw::State& state
);

IMPORT void CheckBlock(
    const mw::BlockRef& block,
    const std::vector<mw::PegIn>& pegInCoins,
    const std::vector<mw::PegOut>& pegOutCoins
);

IMPORT void ConnectBlock(const mw::BlockRef& block, const mw::CoinsViewRef& view);
IMPORT void DisconnectBlock(const mw::BlockRef& block, const mw::CoinsViewRef& view);
IMPORT mw::BlockRef BuildNextBlock(
    const std::vector<mw::TxRef>& transactions,
    const std::vector<mw::PegIn>& pegInCoins,
    const std::vector<mw::PegOut>& pegOutCoins
);

// State
IMPORT mw::StateRef DeserializeState(std::vector<uint8_t>&& bytes);
IMPORT std::vector<uint8_t> SerializeState(const mw::StateRef& state);
IMPORT mw::StateRef SnapshotState(const mw::CoinsViewRef& view, const mw::BlockHash& block_hash);

// Mempool
IMPORT void CheckTransaction(const mw::TxRef& transaction);

END_NAMESPACE // node
END_NAMESPACE // mw