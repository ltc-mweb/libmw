#pragma once

#include <libmw/interfaces/chain_interface.h>
#include <libmw/interfaces/wallet_interface.h>

#include <mw/models/block/Block.h>
#include <mw/models/tx/Transaction.h>
#include <mw/models/crypto/PublicKey.h>
#include <mw/models/crypto/Bech32Address.h>
#include <mw/models/wallet/PartialTx.h>
#include <mw/models/wallet/StealthAddress.h>

class Wallet
{
public:
    Wallet(const libmw::IWallet::Ptr& pWalletInterface, SecretKey&& scan_secret, SecretKey&& spend_secret)
        : m_pWalletInterface(pWalletInterface), m_scanSecret(std::move(scan_secret)), m_spendSecret(std::move(spend_secret)) { }

    static Wallet Open(const libmw::IWallet::Ptr& pWalletInterface);
    
    mw::Transaction::CPtr CreatePegInTx(
        const uint64_t amount,
        const boost::optional<StealthAddress>& receiver_addr
    );

    mw::Transaction::CPtr CreatePegOutTx(
        const uint64_t amount,
        const uint64_t fee_base,
        const Bech32Address& address
    );

    mw::Transaction::CPtr Send(
        const uint64_t amount,
        const uint64_t fee_base,
        const StealthAddress& receiver_address
    );

    mw::Transaction::CPtr CreateTx(
        const std::vector<Commitment>& input_commits,
        const std::vector<std::pair<uint64_t, StealthAddress>>& recipients, // Includes pegins & change
        const std::vector<PegOutCoin>& pegouts,
        const boost::optional<uint64_t>& pegin_amount,
        const uint64_t fee
    ) const;

    StealthAddress GetStealthAddress(const uint32_t index) const;
    StealthAddress GetChangeAddress() const { return GetStealthAddress(libmw::CHANGE_INDEX); }
    StealthAddress GetPegInAddress() const { return GetStealthAddress(libmw::PEGIN_INDEX); }

    libmw::WalletBalance GetBalance() const;

    void BlockConnected(const mw::Block::CPtr& pBlock, const mw::Hash& canonical_block_hash);
    void BlockDisconnected(const mw::Block::CPtr& pBlock);
    void TransactionAddedToMempool(const mw::Transaction::CPtr& pTx);
    void ScanForOutputs(const libmw::IChain::Ptr& pChain);

    std::vector<libmw::Coin> GetCoins(const std::vector<Commitment>& commitments) const
    {
        std::vector<libmw::Coin> coins;
        std::transform(
            commitments.cbegin(), commitments.cend(),
            std::back_inserter(coins),
            [this](const Commitment& commitment) {
                return m_pWalletInterface->GetCoin(commitment.array());
            }
        );

        return coins;
    }

    libmw::IWallet::Ptr GetInterface() const noexcept { return m_pWalletInterface; }

    libmw::Coin RewindOutput(const Output& output) const;

private:
    SecretKey GetSpendKey(const uint32_t index) const;
    bool IsSpendPubKey(const PublicKey& spend_pubkey, uint32_t& index_out) const;

    libmw::IWallet::Ptr m_pWalletInterface;
    SecretKey m_scanSecret;
    SecretKey m_spendSecret;
};