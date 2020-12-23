#include <mw/wallet/Wallet.h>
#include <mw/wallet/OutputFactory.h>
#include <mw/crypto/Blinds.h>
#include <mw/crypto/Bulletproofs.h>
#include <mw/crypto/Keys.h>
#include <mw/crypto/Random.h>
#include <mw/config/ChainParams.h>

#include "CoinSelection.h"
#include "KernelFactory.h"
#include "WalletUtil.h"

Wallet Wallet::Open(const libmw::IWallet::Ptr& pWalletInterface)
{
    assert(pWalletInterface != nullptr);

    SecretKey master_key(pWalletInterface->GetHDKey("m/1/0/0").keyBytes);
    PublicKey master_pub(Crypto::CalculatePublicKey(master_key));
    return Wallet(pWalletInterface, std::move(master_key), std::move(master_pub));
}

mw::Transaction::CPtr Wallet::CreatePegInTx(const uint64_t amount)
{
    libmw::PrivateKey private_key = m_pWalletInterface->GenerateNewHDKey();
    BlindingFactor kernel_offset = Random::CSPRNG<32>();
    BlindingFactor owner_offset = private_key.keyBytes; // TODO: Confirm this

    BlindingFactor blind = Random().CSPRNG<32>();
    Output output = CreateOutput(
        amount,
        EOutputFeatures::PEGGED_IN,
        blind,
        private_key,
        GetStealthAddress()
    );

    BlindingFactor kernel_blind = Blinds()
        .Add(blind)
        .Sub(kernel_offset)
        .Total();
    Kernel kernel = KernelFactory::CreatePegInKernel(kernel_blind, amount);

    libmw::Coin pegin_coin{
        EOutputFeatures::PEGGED_IN,
        private_key.keyBytes,
        blind.array(),
        amount,
        output.GetCommitment().array(),
        boost::none,
        false,
        boost::none
    };
    m_pWalletInterface->AddCoins({ std::move(pegin_coin) });

    TxBody body(
        std::vector<Input>{},
        std::vector<Output>{ std::move(output) },
        std::vector<Kernel>{ std::move(kernel) },
        std::vector<SignedMessage>{} // TODO: Should we split the offset for peg-ins?
    );
    return std::make_shared<mw::Transaction>(
        std::move(kernel_offset),
        std::move(owner_offset),
        std::move(body)
    );
}

mw::Transaction::CPtr Wallet::CreatePegOutTx(
    const uint64_t amount,
    const uint64_t fee_base,
    const Bech32Address& address)
{
    std::vector<libmw::Coin> coins = m_pWalletInterface->ListCoins();

    std::vector<libmw::Coin> input_coins = CoinSelection::SelectCoins(coins, amount, fee_base);
    uint64_t inputs_amount = WalletUtil::TotalAmount(input_coins);
    const uint64_t fee = WalletUtil::CalculateFee(fee_base, input_coins.size(), 1, 2);
    const uint64_t change_amount = inputs_amount - (amount + fee);

    libmw::PrivateKey sender_key = m_pWalletInterface->GenerateNewHDKey();
    BlindingFactor blind = Random().CSPRNG<32>();
    Output change_output = CreateOutput(
        change_amount,
        EOutputFeatures::DEFAULT_OUTPUT,
        blind,
        sender_key,
        GetStealthAddress()
    );

    std::vector<BlindingFactor> input_keys = WalletUtil::GetKeys(input_coins);
    BlindingFactor owner_offset = Blinds().Sub(input_keys).Total();

    BlindingFactor kernel_offset = Random::CSPRNG<32>();
    std::vector<BlindingFactor> input_blinds = WalletUtil::GetBlindingFactors(input_coins);
    BlindingFactor kernel_blind = Blinds()
        .Add(blind)
        .Sub(input_blinds)
        .Sub(kernel_offset)
        .Total();
    Kernel kernel = KernelFactory::CreatePegOutKernel(kernel_blind, amount, fee, address);

    std::vector<libmw::Coin> coins_to_update;
    std::transform(
        input_coins.cbegin(), input_coins.cend(),
        std::back_inserter(coins_to_update),
        [](libmw::Coin input_coin) {
            input_coin.spent = true;
            return input_coin;
        }
    );
    libmw::Coin change_coin{
        EOutputFeatures::DEFAULT_OUTPUT,
        sender_key.keyBytes,
        blind.array(),
        change_amount,
        change_output.GetCommitment().array(),
        boost::none,
        false,
        boost::none
    };
    coins_to_update.push_back(std::move(change_coin));
    m_pWalletInterface->AddCoins(coins_to_update);

    std::vector<Input> inputs = WalletUtil::SignInputs(input_coins);

    TxBody body(
        inputs,
        std::vector<Output>{ std::move(change_output) },
        std::vector<Kernel>{ std::move(kernel) },
        std::vector<SignedMessage>{} // TODO: Need to split the offset for peg-outs
    );
    return std::make_shared<mw::Transaction>(
        std::move(kernel_offset),
        std::move(owner_offset),
        std::move(body)
    );
}

mw::Transaction::CPtr Wallet::Send(
    const uint64_t amount,
    const uint64_t fee_base,
    const StealthAddress& receiver_address)
{
    std::vector<libmw::Coin> coins = m_pWalletInterface->ListCoins();

    std::vector<libmw::Coin> input_coins = CoinSelection::SelectCoins(coins, amount, fee_base);
    uint64_t inputs_amount = WalletUtil::TotalAmount(input_coins);
    const uint64_t fee = WalletUtil::CalculateFee(fee_base, input_coins.size(), 1, 2);

    // Create receiver's output
    BlindingFactor receiver_blind = Random().CSPRNG<32>();
    libmw::PrivateKey ephemeral_key = m_pWalletInterface->GenerateNewHDKey();
    Output receiver_output = CreateOutput(
        amount,
        EOutputFeatures::DEFAULT_OUTPUT,
        receiver_blind,
        ephemeral_key,
        receiver_address
    );

    // Create change output
    const uint64_t change_amount = inputs_amount - (amount + fee);
    BlindingFactor change_blind = Random().CSPRNG<32>();
    libmw::PrivateKey change_key = m_pWalletInterface->GenerateNewHDKey();
    Output change_output = CreateOutput(
        change_amount,
        EOutputFeatures::DEFAULT_OUTPUT,
        change_blind,
        change_key,
        GetStealthAddress()
    );

    std::vector<BlindingFactor> input_blinds = WalletUtil::GetBlindingFactors(input_coins);
    BlindingFactor kernel_offset = Random::CSPRNG<32>();
    BlindingFactor kernel_blind = Blinds()
        .Add(receiver_blind)
        .Add(change_blind)
        .Sub(input_blinds)
        .Sub(kernel_offset)
        .Total();

    Kernel kernel = KernelFactory::CreatePlainKernel(kernel_blind, fee);

    std::vector<Input> inputs = WalletUtil::SignInputs(input_coins);

    std::vector<libmw::Coin> coins_to_update;
    std::transform(
        input_coins.cbegin(), input_coins.cend(),
        std::back_inserter(coins_to_update),
        [](libmw::Coin input_coin) {
            input_coin.spent = true;
            return input_coin;
        }
    );
    libmw::Coin change_coin{
        EOutputFeatures::DEFAULT_OUTPUT,
        change_key.keyBytes,
        change_blind.array(),
        change_amount,
        change_output.GetCommitment().array(),
        boost::none,
        false,
        boost::none
    };
    coins_to_update.push_back(std::move(change_coin));

    m_pWalletInterface->AddCoins(coins_to_update);

    std::vector<BlindingFactor> input_keys = WalletUtil::GetKeys(input_coins);
    BlindingFactor owner_offset = Blinds()
        .Add(ephemeral_key.keyBytes)
        .Add(change_key.keyBytes)
        .Sub(input_keys)
        .Total();

    TxBody body(
        std::move(inputs),
        std::vector<Output>{ std::move(receiver_output), std::move(change_output) },
        std::vector<Kernel>{ std::move(kernel) },
        std::vector<SignedMessage>{} // TODO: Should we split the offset for plain txs?
    );
    return std::make_shared<mw::Transaction>(
        std::move(kernel_offset),
        std::move(owner_offset),
        std::move(body)
    );
}

StealthAddress Wallet::GetStealthAddress() const
{
    SecretKey scan_secret(m_pWalletInterface->GetHDKey("m/1/0/100'").keyBytes);
    SecretKey spend_secret(m_pWalletInterface->GetHDKey("m/1/0/101'").keyBytes);

    return StealthAddress(
        Keys::From(scan_secret).PubKey(),
        Keys::From(spend_secret).PubKey()
    );
}

libmw::MWEBAddress Wallet::GetAddress() const
{
    StealthAddress stealth_address = GetStealthAddress();
    std::vector<uint8_t> serialized = stealth_address.Serialized();
    return EncodeBase32(serialized.data(), serialized.size()); // TODO: Determine encoding
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

        // TODO: Also calculate watch-only balances

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
            PublicKey pubnonce = Keys::From(output.GetOwnerData().GetPubNonce()).Mul(scan_secret).PubKey();
            PublicKey receiver_pubkey = Keys::From(Hashed(pubnonce)).Add(spend_pubkey).PubKey();
            if (receiver_pubkey == output.GetOwnerData().GetReceiverPubKey()) {
                // Output is owned by wallet
                PublicKey ecdh_pubkey = Keys::From(output.GetOwnerData().GetSenderPubKey()).Mul(spend_secret).PubKey();
                SecretKey shared_secret = Hashed(ecdh_pubkey);

                std::vector<uint8_t> decrypted;
                if (output.GetOwnerData().TryDecrypt(shared_secret, decrypted)) {
                    Deserializer deserializer(decrypted);
                    BlindingFactor blind = BlindingFactor::Deserialize(deserializer);
                    uint64_t amount = deserializer.Read<uint64_t>();
                    SecretKey private_key = Crypto::AddPrivateKeys(Hashed(pubnonce), spend_secret);

                    auto iter = coinmap.find(output.GetCommitment());
                    if (iter != coinmap.cend()) {
                        libmw::Coin coin = iter->second;
                        coin.included_block = canonical_block_hash.ToArray();
                        coins_to_update.push_back(std::move(coin));
                    } else {
                        libmw::Coin coin{
                            output.GetFeatures(),
                            private_key.array(),
                            blind.array(),
                            amount,
                            output.GetCommitment().array(),
                            canonical_block_hash.ToArray(),
                            false,
                            boost::none
                        };
                        coins_to_update.push_back(std::move(coin));
                    }
                }
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
    //std::vector<libmw::Coin> orig_coins = m_pWalletInterface->ListCoins();
    //std::cout << "Original coins size: " << orig_coins.size() << std::endl;
    //m_pWalletInterface->DeleteCoins(orig_coins);

    //std::vector<libmw::Coin> coins_to_update;
    //std::unordered_map<Commitment, libmw::Coin&> coinmap;

    //auto pChainIter = pChain->NewIterator();
    //while (pChainIter->Valid()) {
    //    try {
    //        libmw::BlockRef block_ref = pChainIter->GetBlock();
    //        if (block_ref.IsNull()) {
    //            // TODO: Use output mmr
    //        } else {
    //            for (const Output& output : block_ref.pBlock->GetOutputs()) {
    //                try {
    //                    SecretKey nonce = RewindNonce(output.GetCommitment());
    //                    auto pRewound = Bulletproofs::Rewind(
    //                        output.GetCommitment(),
    //                        *output.GetRangeProof(),
    //                        output.GetOwnerData().Serialized(),
    //                        nonce
    //                    );
    //                    if (pRewound == nullptr) {
    //                        continue;
    //                    }

    //                    libmw::PrivateKey private_key = m_pWalletInterface->GetHDKey(
    //                        pRewound->GetKeyChainPath().Format()
    //                    );

    //                    libmw::Coin coin{
    //                        output.GetFeatures(),
    //                        boost::make_optional<libmw::PrivateKey>(std::move(private_key)),
    //                        pRewound->GetAmount(),
    //                        output.GetCommitment().array(),
    //                        boost::make_optional<libmw::BlockHash>(pChainIter->GetCanonicalHash()),
    //                        false,
    //                        boost::none
    //                    };

    //                    std::cout << "Found output " << output.GetCommitment().ToHex() << " - Spent: " << coin.spent_block.has_value() << std::endl;

    //                    coins_to_update.push_back(std::move(coin));
    //                    coinmap.insert({ output.GetCommitment(), coins_to_update.back() });
    //                }
    //                catch (std::exception&) {}
    //            }

    //            for (const Input& input : block_ref.pBlock->GetInputs()) {
    //                auto pCoinIter = coinmap.find(input.GetCommitment());
    //                if (pCoinIter != coinmap.end()) {
    //                    pCoinIter->second.spent = true;
    //                    pCoinIter->second.spent_block = pChainIter->GetCanonicalHash();
    //                }
    //            }
    //        }
    //    } catch (std::exception&) {}

    //    pChainIter->Next();
    //}

    //std::cout << "Adding coins: " << coins_to_update.size() << std::endl;
    //m_pWalletInterface->AddCoins(coins_to_update);
}

Output Wallet::CreateOutput(
    const uint64_t amount,
    const EOutputFeatures features,
    const BlindingFactor& blind,
    const libmw::PrivateKey& sender_privkey,
    const StealthAddress& address) const
{
    Commitment commitment = Crypto::CommitBlinded(amount, blind);
    return OutputFactory::Create(
        features,
        blind,
        SecretKey(sender_privkey.keyBytes),
        address,
        RewindNonce(commitment),
        ProofMessage::FromKeyChain(KeyChainPath::FromString(sender_privkey.bip32Path)),
        amount
    );
}

SecretKey Wallet::RewindNonce(const Commitment& commitment) const
{
    return Hashed(Serializer().Append(commitment).Append(m_masterPub).vec());
}

SecretKey Wallet::SecretNonce(const Commitment& commitment) const
{
    return Hashed(Serializer().Append(commitment).Append(m_masterKey).vec());
}