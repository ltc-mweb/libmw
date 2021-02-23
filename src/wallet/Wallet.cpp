#include <mw/wallet/Wallet.h>
#include <mw/crypto/Keys.h>
#include <mw/crypto/Random.h>
#include <mw/config/ChainParams.h>
#include <mw/exceptions/InsufficientFundsException.h>

#include "Transact.h"

Wallet Wallet::Open(const libmw::IWallet::Ptr& pWalletInterface)
{
    assert(pWalletInterface != nullptr);

    SecretKey scan_secret(pWalletInterface->GetHDKey("m/1/0/100'").keyBytes);
    SecretKey spend_secret(pWalletInterface->GetHDKey("m/1/0/101'").keyBytes);

    return Wallet(pWalletInterface, std::move(scan_secret), std::move(spend_secret));
}

mw::Transaction::CPtr Wallet::CreateTx(
    const std::vector<Commitment>& input_commits,
    const std::vector<std::pair<uint64_t, StealthAddress>>& recipients,
    const std::vector<PegOutCoin>& pegouts,
    const boost::optional<uint64_t>& pegin_amount,
    const uint64_t fee) const
{
    return Transact(*this).CreateTx(input_commits, recipients, pegouts, pegin_amount, fee);
}

bool Wallet::CommitTx(const mw::Transaction::CPtr& pTransaction)
{
    // TODO: Implement
    return false;
}

StealthAddress Wallet::GetStealthAddress(const uint32_t index) const
{
    PublicKey Bi = PublicKey::From(GetSpendKey(index));
    PublicKey Ai = Bi.Mul(m_scanSecret); 

    return StealthAddress(Ai, Bi);
}

SecretKey Wallet::GetSpendKey(const uint32_t index) const
{
    SecretKey mi = Hasher(EHashTag::ADDRESS)
        .Append<uint32_t>(index)
        .Append(m_scanSecret)
        .hash();

    return Crypto::AddPrivateKeys(m_spendSecret, mi);
}

bool Wallet::IsSpendPubKey(const PublicKey& spend_pubkey, uint32_t& index_out) const
{
    if (GetChangeAddress().B() == spend_pubkey) {
        index_out = libmw::CHANGE_INDEX;
        return true;
    }

    if (GetPegInAddress().B() == spend_pubkey) {
        index_out = libmw::PEGIN_INDEX;
        return true;
    }

    // TODO: Check all receive addresses (Need to use a key cache for performance)
    if (GetStealthAddress(0).B() == spend_pubkey) {
        index_out = 0;
        return true;
    }

    return false;
}

libmw::WalletBalance Wallet::GetBalance() const
{
    libmw::WalletBalance balance;

    std::vector<libmw::Coin> coins = m_pWalletInterface->ListCoins();
    for (const libmw::Coin& coin : coins) {
        if (coin.spent_block.has_value()) {
            continue;
        }

        uint64_t num_confirmations = 0;
        if (coin.included_block.has_value()) {
            num_confirmations = m_pWalletInterface->GetDepthInActiveChain(coin.included_block.value());
        }

        // FUTURE: Also calculate watch-only balances

        if (num_confirmations == 0) {
            balance.unconfirmed_balance += coin.amount;
        } else if (coin.spent) {
            balance.locked_balance += coin.amount;
        } else if ((coin.features & libmw::PEGIN_OUTPUT) == 0 || num_confirmations >= mw::ChainParams::GetPegInMaturity()) {
            balance.confirmed_balance += coin.amount;
        } else {
            balance.immature_balance += coin.amount;
        }
    }

    return balance;
}

void Wallet::BlockConnected(const mw::Block::CPtr& pBlock, const mw::Hash& canonical_block_hash)
{
    std::vector<libmw::Coin> coins_to_update;

    std::vector<libmw::Coin> coinlist = m_pWalletInterface->ListCoins();
    std::unordered_map<Commitment, libmw::Coin> coinmap;
    std::transform(
        coinlist.cbegin(), coinlist.cend(),
        std::inserter(coinmap, coinmap.end()),
        [](const libmw::Coin& coin) { return std::make_pair(Commitment(coin.commitment), coin); }
    );

    // Mark outputs as confirmed
    PublicKey spend_pubkey = Keys::From(m_spendSecret).PubKey();
    for (const Output& output : pBlock->GetOutputs()) {
        try {
            auto iter = coinmap.find(output.GetCommitment());
            if (iter != coinmap.cend()) {
                libmw::Coin coin = iter->second;
                coin.included_block = canonical_block_hash.ToArray();
                coins_to_update.push_back(std::move(coin));
            } else {
                libmw::Coin coin = RewindOutput(output);
                coin.included_block = canonical_block_hash.ToArray();
                coins_to_update.push_back(std::move(coin));
            }
        } catch (std::exception&) { }
    }

    // Mark inputs as spent
    for (const Input& input : pBlock->GetInputs()) {
        auto iter = coinmap.find(input.GetCommitment());
        if (iter != coinmap.cend()) {
            libmw::Coin coin = iter->second;
            coin.spent = true;
            coin.spent_block = canonical_block_hash.ToArray();
            coins_to_update.push_back(coin);
        }
    }

    m_pWalletInterface->AddCoins(coins_to_update);
}

void Wallet::BlockDisconnected(const mw::Block::CPtr& pBlock)
{
    std::vector<libmw::Coin> coins_to_update;

    std::vector<libmw::Coin> coinlist = m_pWalletInterface->ListCoins();
    std::unordered_map<Commitment, libmw::Coin> coinmap;
    std::transform(
        coinlist.cbegin(), coinlist.cend(),
        std::inserter(coinmap, coinmap.end()),
        [](const libmw::Coin& coin) { return std::make_pair(Commitment(coin.commitment), coin); }
    );

    // Mark outputs as unconfirmed
    for (const Output& output : pBlock->GetOutputs()) {
        auto iter = coinmap.find(output.GetCommitment());
        if (iter != coinmap.cend()) {
            libmw::Coin coin = iter->second;
            coin.included_block = boost::none;
            coins_to_update.push_back(coin);
        }
    }

    // Mark inputs as unspent
    for (const Input& input : pBlock->GetInputs()) {
        auto iter = coinmap.find(input.GetCommitment());
        if (iter != coinmap.cend()) {
            libmw::Coin coin = iter->second;
            coin.spent = true;
            coin.spent_block = boost::none;
            coins_to_update.push_back(coin);
        }
    }

    m_pWalletInterface->AddCoins(coins_to_update);
}

void Wallet::TransactionAddedToMempool(const mw::Transaction::CPtr& pTx)
{
    std::vector<libmw::Coin> coins_to_update;

    std::vector<libmw::Coin> coinlist = m_pWalletInterface->ListCoins();
    std::unordered_map<Commitment, libmw::Coin> coinmap;
    std::transform(
        coinlist.cbegin(), coinlist.cend(),
        std::inserter(coinmap, coinmap.end()),
        [](const libmw::Coin& coin) { return std::make_pair(Commitment(coin.commitment), coin); }
    );

    // Mark outputs as confirmed
    PublicKey spend_pubkey = Keys::From(m_spendSecret).PubKey();
    for (const Output& output : pTx->GetOutputs()) {
        try {
            auto iter = coinmap.find(output.GetCommitment());
            if (iter == coinmap.cend()) {
                coins_to_update.push_back(RewindOutput(output));
            }
        }
        catch (std::exception&) {}
    }

    // Mark inputs as spent
    for (const Input& input : pTx->GetInputs()) {
        auto iter = coinmap.find(input.GetCommitment());
        if (iter != coinmap.cend()) {
            libmw::Coin coin = iter->second;
            coin.spent = true;
            coins_to_update.push_back(coin);
        }
    }

    m_pWalletInterface->AddCoins(coins_to_update);
}

void Wallet::ScanForOutputs(const libmw::IChain::Ptr& pChain)
{
    std::vector<libmw::Coin> orig_coins = m_pWalletInterface->ListCoins();
    m_pWalletInterface->DeleteCoins(orig_coins);

    std::vector<libmw::Coin> coins_to_update;
    std::unordered_map<Commitment, libmw::Coin&> coinmap;

    auto pChainIter = pChain->NewIterator();
    while (pChainIter->Valid()) {
        try {
            libmw::BlockRef block_ref = pChainIter->GetBlock();
            if (block_ref.IsNull()) {
                // TODO: Use output mmr
            } else {
                for (const Output& output : block_ref.pBlock->GetOutputs()) {
                    try {
                        libmw::Coin coin = RewindOutput(output);
                        coin.included_block = boost::make_optional<libmw::BlockHash>(pChainIter->GetCanonicalHash());
                        if (coin.address_index != libmw::CHANGE_INDEX && !Features(coin.features).IsSet(EOutputFeatures::PEGGED_IN)) {
                            // TODO: Create CWalletTx
                        }
                    }
                    catch (std::exception&) {}
                }

                for (const Input& input : block_ref.pBlock->GetInputs()) {
                    auto pCoinIter = coinmap.find(input.GetCommitment());
                    if (pCoinIter != coinmap.end()) {
                        pCoinIter->second.spent = true;
                        pCoinIter->second.spent_block = pChainIter->GetCanonicalHash();
                    }
                }
            }
        } catch (std::exception&) {}

        pChainIter->Next();
    }

    //std::cout << "Adding coins: " << coins_to_update.size() << std::endl;
    m_pWalletInterface->AddCoins(coins_to_update);
}

libmw::Coin Wallet::RewindOutput(const Output& output) const
{
    SecretKey a(m_pWalletInterface->GetHDKey("m/1/0/100'").keyBytes);

    SecretKey t = Hashed(EHashTag::DERIVE, output.Ke().Mul(a));
    if (t[0] == output.GetViewTag()) {
        PublicKey B = output.Ko().Sub(Hashed(EHashTag::OUT_KEY, t));

        // Check if B belongs to wallet
        uint32_t index = 0;
        if (IsSpendPubKey(B, index)) {
            StealthAddress wallet_addr = GetStealthAddress(index);
            Deserializer hash64(Hash512(t).vec());
            SecretKey r = hash64.Read<SecretKey>();
            uint64_t value = output.GetMaskedValue() ^ hash64.Read<uint64_t>();
            BigInt<16> n = output.GetMaskedNonce() ^ hash64.ReadVector(16);

            if (Commitment::Switch(r, value) == output.GetCommitment()) {
                // Calculate Carol's sending key 's' and check that s*B ?= Ke
                SecretKey s = Hasher(EHashTag::SEND_KEY)
                    .Append(wallet_addr.A())
                    .Append(wallet_addr.B())
                    .Append(value)
                    .Append(n)
                    .hash();
                if (output.Ke() == wallet_addr.B().Mul(s)) {
                    SecretKey private_key = Crypto::AddPrivateKeys(
                        Hashed(EHashTag::OUT_KEY, t),
                        GetSpendKey(index)
                    );
                    return libmw::Coin{
                        output.GetFeatures().Get(),
                        index,
                        private_key.array(),
                        r.array(),
                        value,
                        output.GetCommitment().array(),
                        boost::none,
                        false,
                        boost::none,
                        time(nullptr)
                    };
                }
            }
        }
    }

    throw std::runtime_error("Unable to rewind output");
}