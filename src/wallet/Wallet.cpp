#include <mw/wallet/Wallet.h>
#include <mw/crypto/Keys.h>
#include <mw/crypto/Random.h>
#include <mw/config/ChainParams.h>
#include <mw/exceptions/InsufficientFundsException.h>

#include "PegIn.h"
#include "PegOut.h"
#include "Transact.h"

Wallet Wallet::Open(const libmw::IWallet::Ptr& pWalletInterface)
{
    assert(pWalletInterface != nullptr);

    SecretKey master_key(pWalletInterface->GetHDKey("m/1/0/0").keyBytes);
    PublicKey master_pub(Crypto::CalculatePublicKey(master_key.GetBigInt()));
    return Wallet(pWalletInterface, std::move(master_key), std::move(master_pub));
}

mw::Transaction::CPtr Wallet::CreatePegInTx(const uint64_t amount, const boost::optional<StealthAddress>& receiver_addr)
{
    return PegIn(*this).CreatePegInTx(amount, receiver_addr.value_or(GetStealthAddress()));
}

mw::Transaction::CPtr Wallet::CreatePegOutTx(
    const uint64_t amount,
    const uint64_t fee_base,
    const Bech32Address& address)
{
    return PegOut(*this).CreatePegOutTx(amount, fee_base, address);
}

mw::Transaction::CPtr Wallet::Send(
    const uint64_t amount,
    const uint64_t fee_base,
    const StealthAddress& receiver_address)
{
    return Transact(*this).CreateTx(amount, fee_base, receiver_address);
}

StealthAddress Wallet::GetStealthAddress(const uint32_t index) const
{
    SecretKey a(m_pWalletInterface->GetHDKey("m/1/0/100'").keyBytes);
    SecretKey b(m_pWalletInterface->GetHDKey("m/1/0/101'").keyBytes);
    SecretKey mi = Hasher(EHashTag::ADDRESS)
        .Append<uint32_t>(index)
        .Append(a)
        .hash();

    PublicKey Bi = Crypto::CalculatePublicKey(Crypto::AddPrivateKeys(b, mi).GetBigInt());
    PublicKey Ai = Crypto::MultiplyKey(Bi, a);

    return StealthAddress(Ai, Bi);
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
    SecretKey scan_secret(m_pWalletInterface->GetHDKey("m/1/0/100'").keyBytes);
    SecretKey spend_secret(m_pWalletInterface->GetHDKey("m/1/0/101'").keyBytes);
    PublicKey spend_pubkey = Keys::From(spend_secret).PubKey();
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

// TODO: Implement
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
                        if (!coin.change_output) {

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
    //// Mark outputs as confirmed
    //SecretKey scan_secret(m_pWalletInterface->GetHDKey("m/1/0/100'").keyBytes);
    //SecretKey spend_secret(m_pWalletInterface->GetHDKey("m/1/0/101'").keyBytes);

    //PublicKey pubnonce = Keys::From(output.GetOwnerData().GetPubNonce()).Mul(scan_secret).PubKey();
    //PublicKey receiver_pubkey = Keys::From(Hashed(pubnonce)).Add(spend_secret).PubKey();
    //if (receiver_pubkey == output.GetOwnerData().GetReceiverPubKey()) {
    //    // Output is owned by wallet
    //    PublicKey ecdh_pubkey = Keys::From(output.GetOwnerData().GetSenderPubKey()).Mul(spend_secret).PubKey();
    //    SecretKey shared_secret = Hashed(ecdh_pubkey);

    //    std::vector<uint8_t> decrypted;
    //    if (output.GetOwnerData().TryDecrypt(shared_secret, decrypted)) {
    //        Deserializer deserializer(decrypted);
    //        BlindingFactor blind = BlindingFactor::Deserialize(deserializer);
    //        uint64_t amount = deserializer.Read<uint64_t>();
    //        SecretKey private_key = Crypto::AddPrivateKeys(Hashed(pubnonce), spend_secret);
    //        bool change_output = false; // TODO: Make this a bitmask of private features instead?
    //        if (deserializer.GetRemainingSize() > 0) {
    //            change_output = deserializer.Read<bool>();
    //        }

    //        return libmw::Coin{
    //            output.GetFeatures(),
    //            change_output,
    //            private_key.array(),
    //            blind.array(),
    //            amount,
    //            output.GetCommitment().array(),
    //            boost::none,
    //            false,
    //            boost::none
    //        };
    //    }
    //}

    throw std::runtime_error("Unable to rewind output");
}