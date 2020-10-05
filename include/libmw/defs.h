#pragma once

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

#define LIBMW_NAMESPACE namespace libmw {
#define NODE_NAMESPACE namespace node {
#define DB_NAMESPACE namespace db {
#define WALLET_NAMESPACE namespace wallet {
#define END_NAMESPACE }

// Forward Declarations
namespace mw
{
    class Header;
    class Block;
    class BlockUndo;
    class Transaction;
    class ICoinsView;
    struct State;
}

LIBMW_NAMESPACE

typedef std::array<uint8_t, 32> BlockHash;
typedef std::array<uint8_t, 32> Offset;
typedef std::array<uint8_t, 32> BlindingFactor;
typedef std::array<uint8_t, 33> Commitment;

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
    IMPORT libmw::HeaderRef GetHeader() const;

    std::shared_ptr<mw::Block> pBlock;
};

struct BlockUndoRef
{
    std::shared_ptr<const mw::BlockUndo> pUndo;
};

struct TxRef
{
    IMPORT std::vector<PegOut> GetPegouts() const noexcept;
    IMPORT uint64_t GetTotalFee() const noexcept;

    std::shared_ptr<const mw::Transaction> pTransaction;
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

struct ChainParams
{
    std::string dataDirectory;
    std::string hrp;
};

struct PrivateKey
{
    std::string bip32Path;
    libmw::BlindingFactor keyBytes;
};

struct Coin
{
    PrivateKey key;
    uint64_t amount;
    libmw::Commitment commitment;
};

END_NAMESPACE