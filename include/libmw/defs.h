#pragma once

#include <array>
#include <set>
#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include <unordered_map>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#if defined(_WIN32)
//  Microsoft 
#ifdef LIBMW
#define MWEXPORT __declspec(dllexport)
#define MWIMPORT __declspec(dllexport)
#else
#define MWIMPORT __declspec(dllimport)
#endif
#elif defined(__GNUC__)
//  GCC
#define MWEXPORT __attribute__((visibility("default")))
#define MWIMPORT 
#else
#define MWEXPORT
#define MWIMPORT
#endif

#define LIBMW_NAMESPACE namespace libmw {
#define NODE_NAMESPACE namespace node {
#define MINER_NAMESPACE namespace miner {
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
    class BlockBuilder;
    struct State;
}

// TODO: Consider pulling in serialize.h, and adding serialization logic here.

LIBMW_NAMESPACE

typedef std::array<uint8_t, 32> BlockHash;
typedef std::array<uint8_t, 32> KernelHash;
typedef std::array<uint8_t, 32> Offset;
typedef std::array<uint8_t, 32> BlindingFactor;
typedef std::array<uint8_t, 33> Commitment;
typedef std::array<uint8_t, 33> PubKey;
typedef std::string MWEBAddress;

static const uint8_t NORMAL_OUTPUT = 0;
static const uint8_t PEGIN_OUTPUT = 1;

static constexpr size_t MAX_BLOCK_WEIGHT = 21'000;
static constexpr size_t KERNEL_WEIGHT = 2;
static constexpr size_t OWNER_SIG_WEIGHT = 1;
static constexpr size_t OUTPUT_WEIGHT = 18;
static constexpr uint16_t PEGIN_MATURITY = 20;
static constexpr uint8_t MAX_KERNEL_EXTRADATA_SIZE = 33;

struct PegIn
{
    uint64_t amount;
    std::array<uint8_t, 33> commitment;

    bool operator==(const PegIn& rhs) const
    {
        return amount == rhs.amount && commitment == rhs.commitment;
    }

    bool operator!=(const PegIn& rhs) const
    {
        return !(*this == rhs);
    }
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
    bool IsNull() const noexcept { return pBlock == nullptr; }

    MWIMPORT libmw::BlockHash GetHash() const noexcept;
    MWIMPORT libmw::HeaderRef GetHeader() const;
    MWIMPORT uint64_t GetTotalFee() const noexcept;
    MWIMPORT uint64_t GetWeight() const noexcept;
    MWIMPORT std::set<KernelHash> GetKernelHashes() const;
    MWIMPORT std::vector<libmw::Commitment> GetInputCommits() const;
    MWIMPORT std::vector<libmw::Commitment> GetOutputCommits() const;

    std::shared_ptr<mw::Block> pBlock;
};

struct BlockAndPegs
{
    std::shared_ptr<mw::Block> pBlock;
    std::vector<PegIn> pegins;
    std::vector<PegOut> pegouts;
};

struct BlockUndoRef
{
    std::shared_ptr<const mw::BlockUndo> pUndo;
};

struct TxRef
{
    MWIMPORT std::vector<PegOut> GetPegouts() const noexcept;
    MWIMPORT std::vector<PegIn> GetPegins() const noexcept;
    MWIMPORT uint64_t GetTotalFee() const noexcept;
    MWIMPORT uint64_t GetWeight() const noexcept;
    MWIMPORT std::set<KernelHash> GetKernelHashes() const noexcept;
    MWIMPORT std::set<libmw::Commitment> GetInputCommits() const noexcept;
    MWIMPORT std::set<libmw::Commitment> GetOutputCommits() const noexcept;

    std::shared_ptr<const mw::Transaction> pTransaction;
};

struct CoinsViewRef
{
    //
    // Creates a new CoinsViewCache on top of this CoinsView.
    //
    MWIMPORT CoinsViewRef CreateCache() const;

    std::shared_ptr<mw::ICoinsView> pCoinsView;
};

struct StateRef
{
    std::shared_ptr<mw::State> pState;
};

struct ChainParams
{
    boost::filesystem::path dataDirectory;
    std::string hrp;
};

struct PrivateKey
{
    std::string bip32Path;
    libmw::BlindingFactor keyBytes;
};

struct BlockBuilderRef
{
    std::shared_ptr<mw::BlockBuilder> pBuilder;
};

inline static constexpr uint32_t CHANGE_INDEX{ 2'000'000 };
inline static constexpr uint32_t PEGIN_INDEX{ 4'000'000 };

/// <summary>
/// Represents an output owned by the wallet.
/// </summary>
struct Coin
{
    // 0 for typical outputs or 1 for pegged-in outputs
    // This is used to determine the required number of confirmations before spending.
    uint8_t features;

    // Index of the subaddress this coin was received at.
    uint32_t address_index;

    // The private key needed in order to spend the coin.
    // May be empty for watch-only wallets.
    boost::optional<libmw::BlindingFactor> key;

    // The blinding factor needed in order to spend the coin.
    // May be empty for watch-only wallets.
    boost::optional<libmw::BlindingFactor> blind;

    // The output amount in litoshis.
    // Typically positive, but could be 0 in the future when we start using decoys to improve privacy.
    uint64_t amount;

    // The output commitment (v*H + r*G).
    libmw::Commitment commitment;

    bool IsChange() const noexcept { return address_index == CHANGE_INDEX; }
    bool IsPegIn() const noexcept { return address_index == PEGIN_INDEX; }
};

/// <summary>
/// Contains the balances & statuses of all of the wallet's unspent coins.
/// </summary>
struct WalletBalance
{
    // Confirmed on-chain, meets maturity requirements.
    uint64_t confirmed_balance = 0;

    // Received coins that have not yet been seen on-chain.
    uint64_t unconfirmed_balance = 0;

    // Confirmed on-chain, but does not meet maturity requirements (eg. pegins).
    uint64_t immature_balance = 0;

    // Coins that have been spent, but whose spending txs has not yet been seen on-chain.
    uint64_t locked_balance = 0;
};

END_NAMESPACE