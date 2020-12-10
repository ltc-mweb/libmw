#include <mw/wallet/Wallet.h>
#include <mw/crypto/Blinds.h>
#include <mw/crypto/Random.h>
#include <mw/config/ChainParams.h>

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
    BlindingFactor offset = Random::CSPRNG<32>();

    Output output = CreateOutput(amount, EOutputFeatures::PEGGED_IN, private_key);

    BlindingFactor kernel_blind = Blinds()
        .Add(private_key.keyBytes)
        .Sub(offset)
        .Total();
    Kernel kernel = KernelFactory::CreatePegInKernel(kernel_blind, amount);

    libmw::Coin pegin_coin{
        EOutputFeatures::PEGGED_IN,
        private_key,
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
        std::vector<Kernel>{ std::move(kernel) }
    );
    return std::make_shared<mw::Transaction>(std::move(offset), std::move(body));
}

mw::Transaction::CPtr Wallet::CreatePegOutTx(
    const uint64_t amount,
    const uint64_t fee_base,
    const Bech32Address& address)
{
    std::vector<libmw::Coin> coins = m_pWalletInterface->ListCoins();

    std::vector<libmw::Coin> input_coins = m_pWalletInterface->SelectCoins(coins, amount, fee_base);
    BlindingFactor input_blinds = WalletUtil::AddBlindingFactors(input_coins);
    uint64_t inputs_amount = WalletUtil::TotalAmount(input_coins);
    const uint64_t fee = WalletUtil::CalculateFee(fee_base, input_coins.size(), 1, 2);
    const uint64_t change_amount = inputs_amount - (amount + fee);

    libmw::PrivateKey private_key = m_pWalletInterface->GenerateNewHDKey();
    BlindingFactor blindingFactor(private_key.keyBytes);
    Output change_output = CreateOutput(change_amount, EOutputFeatures::DEFAULT_OUTPUT, private_key);

    BlindingFactor offset = Random::CSPRNG<32>();
    BlindingFactor kernel_blind = Blinds()
        .Add(blindingFactor)
        .Sub(input_blinds)
        .Sub(offset)
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
        private_key,
        change_amount,
        change_output.GetCommitment().array(),
        boost::none,
        false,
        boost::none
    };
    coins_to_update.push_back(std::move(change_coin));
    m_pWalletInterface->AddCoins(coins_to_update);

    std::vector<Input> inputs;
    std::transform(
        input_coins.cbegin(), input_coins.cend(),
        std::back_inserter(inputs),
        [](const libmw::Coin& input_coin) {
            return Input((EOutputFeatures)input_coin.features, Commitment(input_coin.commitment));
        }
    );

    TxBody body(
        inputs,
        std::vector<Output>{ std::move(change_output) },
        std::vector<Kernel>{ std::move(kernel) }
    );
    return std::make_shared<mw::Transaction>(std::move(offset), std::move(body));
}

PartialTx Wallet::Send(const uint64_t amount, const uint64_t fee_base)
{
    std::vector<libmw::Coin> coins = m_pWalletInterface->ListCoins();

    std::vector<libmw::Coin> input_coins = m_pWalletInterface->SelectCoins(coins, amount, fee_base);
    BlindingFactor input_blinds = WalletUtil::AddBlindingFactors(input_coins);
    uint64_t inputs_amount = WalletUtil::TotalAmount(input_coins);
    const uint64_t fee = WalletUtil::CalculateFee(fee_base, input_coins.size(), 1, 2);
    const uint64_t change_amount = inputs_amount - (amount + fee);

    libmw::PrivateKey private_key = m_pWalletInterface->GenerateNewHDKey();
    BlindingFactor change_blind(private_key.keyBytes);
    Output change_output = CreateOutput(change_amount, EOutputFeatures::DEFAULT_OUTPUT, private_key);

    BlindingFactor blind = Blinds()
        .Add(change_blind)
        .Sub(input_blinds)
        .Total();

    std::vector<Input> inputs;
    std::transform(
        input_coins.cbegin(), input_coins.cend(),
        std::back_inserter(inputs),
        [](const libmw::Coin& input_coin) {
            return Input((EOutputFeatures)input_coin.features, Commitment(input_coin.commitment));
        }
    );

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
        private_key,
        change_amount,
        change_output.GetCommitment().array(),
        boost::none,
        false,
        boost::none
    };
    coins_to_update.push_back(std::move(change_coin));

    m_pWalletInterface->AddCoins(coins_to_update);

    return PartialTx(amount, fee, inputs, { change_output }, blind);
}

mw::Transaction::CPtr Wallet::Receive(const PartialTx& partial_tx)
{
    libmw::PrivateKey private_key = m_pWalletInterface->GenerateNewHDKey();
    BlindingFactor received_blind(private_key.keyBytes);
    Output received_output = CreateOutput(partial_tx.GetAmount(), EOutputFeatures::DEFAULT_OUTPUT, private_key);

    BlindingFactor tx_offset = Random::CSPRNG<32>();
    BlindingFactor kernel_blind = Crypto::AddBlindingFactors({ received_blind, partial_tx.GetBlind() }, { tx_offset });
    Kernel kernel = KernelFactory::CreatePlainKernel(kernel_blind, partial_tx.GetFee());

    libmw::Coin received_coin{
        EOutputFeatures::DEFAULT_OUTPUT,
        private_key,
        partial_tx.GetAmount(),
        received_output.GetCommitment().array(),
        boost::none,
        false,
        boost::none
    };
    m_pWalletInterface->AddCoins({ received_coin });

    std::vector<Output> outputs = partial_tx.GetChange();
    outputs.push_back(received_output);
    TxBody body(partial_tx.GetInputs(), outputs, { kernel });

    return std::make_shared<mw::Transaction>(std::move(tx_offset), std::move(body));
}

libmw::MWEBAddress Wallet::GetAddress() const
{
    SecretKey private_key(m_pWalletInterface->GetHDKey("m/1/0/100").keyBytes);
    return Bech32Address("mweb", Crypto::CalculatePublicKey(private_key).vec()).ToString();
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
    BlindingFactor master_key = m_pWalletInterface->GetHDKey("m/1/0/0").keyBytes;
    for (const Output& output : pBlock->GetOutputs()) {
        try {
            SecretKey nonce = RewindNonce(output.GetCommitment());
            auto pRewound = Crypto::RewindRangeProof(output.GetCommitment(), *output.GetRangeProof(), nonce);
            if (pRewound == nullptr) {
                continue;
            }

            auto iter = coinmap.find(output.GetCommitment());
            if (iter != coinmap.cend()) {
                libmw::Coin coin = iter->second;
                coin.included_block = canonical_block_hash.ToArray();
                coins_to_update.push_back(std::move(coin));
            } else {
                libmw::PrivateKey private_key = m_pWalletInterface->GetHDKey(
                    pRewound->GetKeyChainPath().Format()
                );

                libmw::Coin coin{
                    output.GetFeatures(),
                    std::move(private_key),
                    pRewound->GetAmount(),
                    output.GetCommitment().array(),
                    canonical_block_hash.ToArray(),
                    false,
                    boost::none
                };
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

void Wallet::ScanForOutputs(const libmw::IChain::Ptr& pChain)
{
    std::vector<libmw::Coin> orig_coins = m_pWalletInterface->ListCoins();
    std::cout << "Original coins size: " << orig_coins.size() << std::endl;
    m_pWalletInterface->DeleteCoins(orig_coins);

    std::vector<libmw::Coin> coins_to_update;
    std::unordered_map<Commitment, libmw::Coin&> coinmap;

    auto pChainIter = pChain->NewIterator();
    while (pChainIter->Valid()) {
        libmw::BlockRef block_ref = pChainIter->GetBlock();
        if (block_ref.IsNull()) {
            // TODO: Use output mmr
        } else {
            for (const Output& output : block_ref.pBlock->GetOutputs()) {
                try {
                    SecretKey nonce = RewindNonce(output.GetCommitment());
                    auto pRewound = Crypto::RewindRangeProof(output.GetCommitment(), *output.GetRangeProof(), nonce);
                    if (pRewound == nullptr) {
                        continue;
                    }

                    libmw::PrivateKey private_key = m_pWalletInterface->GetHDKey(
                        pRewound->GetKeyChainPath().Format()
                    );

                    libmw::Coin coin{
                        output.GetFeatures(),
                        boost::make_optional<libmw::PrivateKey>(std::move(private_key)),
                        pRewound->GetAmount(),
                        output.GetCommitment().array(),
                        boost::make_optional<libmw::BlockHash>(pChainIter->GetCanonicalHash()),
                        false,
                        boost::none
                    };

                    std::cout << "Found output " << output.GetCommitment().ToHex() << " - Spent: " << coin.spent_block.has_value() << std::endl;

                    coins_to_update.push_back(std::move(coin));
                    coinmap.insert({ output.GetCommitment(), coins_to_update.back() });
                } catch (std::exception&) { }
            }

            for (const Input& input : block_ref.pBlock->GetInputs()) {
                auto pCoinIter = coinmap.find(input.GetCommitment());
                if (pCoinIter != coinmap.end()) {
                    pCoinIter->second.spent = true;
                    pCoinIter->second.spent_block = pChainIter->GetCanonicalHash();
                }
            }
        }

        pChainIter->Next();
    }

    std::cout << "Adding coins: " << coins_to_update.size() << std::endl;
    m_pWalletInterface->AddCoins(coins_to_update);
}

Output Wallet::CreateOutput(
    const uint64_t amount,
    const EOutputFeatures features,
    const libmw::PrivateKey& private_key) const
{
    Commitment commitment = Crypto::CommitBlinded(amount, private_key.keyBytes);

    RangeProof::CPtr pRangeProof = Crypto::GenerateRangeProof(
        amount,
        SecretKey(private_key.keyBytes),
        SecretNonce(commitment),
        RewindNonce(commitment),
        ProofMessage::FromKeyChain(KeyChainPath::FromString(private_key.bip32Path))
    );

    return Output{ features, std::move(commitment), {}, pRangeProof };
}

SecretKey Wallet::RewindNonce(const Commitment& commitment) const
{
    return Hashed(Serializer().Append(commitment).Append(m_masterPub).vec());
}

SecretKey Wallet::SecretNonce(const Commitment& commitment) const
{
    return Hashed(Serializer().Append(commitment).Append(m_masterKey).vec());
}