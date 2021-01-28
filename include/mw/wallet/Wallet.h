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
    Wallet(const libmw::IWallet::Ptr& pWalletInterface, SecretKey&& masterKey, PublicKey&& masterPub)
        : m_pWalletInterface(pWalletInterface), m_masterKey(std::move(masterKey)), m_masterPub(std::move(masterPub)) { }

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

    StealthAddress GetStealthAddress(const uint32_t index = 0) const;
    libmw::WalletBalance GetBalance() const;

    void BlockConnected(const mw::Block::CPtr& pBlock, const mw::Hash& canonical_block_hash);
    void BlockDisconnected(const mw::Block::CPtr& pBlock);
    void ScanForOutputs(const libmw::IChain::Ptr& pChain);

    libmw::IWallet::Ptr GetInterface() const noexcept { return m_pWalletInterface; }
    SecretKey NewKey() const { return m_pWalletInterface->GenerateNewHDKey().keyBytes; }

    libmw::Coin RewindOutput(const Output& output) const;

private:
    libmw::IWallet::Ptr m_pWalletInterface;
    SecretKey m_masterKey;
    PublicKey m_masterPub;
};