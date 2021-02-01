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

    StealthAddress GetStealthAddress(const uint32_t index) const;
    StealthAddress GetChangeAddress() const { return GetStealthAddress(CHANGE_INDEX); }
    StealthAddress GetPegInAddress() const { return GetStealthAddress(PEGIN_INDEX); }

    libmw::WalletBalance GetBalance() const;

    void BlockConnected(const mw::Block::CPtr& pBlock, const mw::Hash& canonical_block_hash);
    void BlockDisconnected(const mw::Block::CPtr& pBlock);
    void ScanForOutputs(const libmw::IChain::Ptr& pChain);

    libmw::IWallet::Ptr GetInterface() const noexcept { return m_pWalletInterface; }
    SecretKey NewKey() const { return m_pWalletInterface->GenerateNewHDKey().keyBytes; }

    libmw::Coin RewindOutput(const Output& output) const;

private:
    SecretKey GetSpendKey(const uint32_t index) const;
    bool IsSpendPubKey(const PublicKey& spend_pubkey, uint32_t& index_out) const;

    libmw::IWallet::Ptr m_pWalletInterface;
    SecretKey m_scanSecret;
    SecretKey m_spendSecret;

    inline static constexpr uint32_t CHANGE_INDEX{ 2'000'000 };
    inline static constexpr uint32_t PEGIN_INDEX{ 4'000'000 };
};