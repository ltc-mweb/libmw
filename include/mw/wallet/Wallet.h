#pragma once

#include <libmw/interfaces/wallet_interface.h>
#include <mw/models/block/Block.h>
#include <mw/models/tx/Transaction.h>
#include <mw/models/crypto/PublicKey.h>
#include <mw/models/crypto/Bech32Address.h>
#include <mw/models/wallet/PartialTx.h>

class Wallet
{
public:
    Wallet(const libmw::IWallet::Ptr& pWalletInterface, SecretKey&& masterKey, PublicKey&& masterPub)
        : m_pWalletInterface(pWalletInterface), m_masterKey(std::move(masterKey)), m_masterPub(std::move(masterPub)) { }

    static Wallet Open(const libmw::IWallet::Ptr& pWalletInterface);
    
    mw::Transaction::CPtr CreatePegInTx(const uint64_t amount);
    mw::Transaction::CPtr CreatePegOutTx(
        const uint64_t amount,
        const uint64_t fee_base,
        const Bech32Address& address
    );

    PartialTx Send(const uint64_t amount, const uint64_t fee_base);
    mw::Transaction::CPtr Receive(const PartialTx& partial_tx);

    libmw::MWEBAddress GetAddress() const;
    libmw::WalletBalance GetBalance() const;

    void BlockConnected(const mw::Block::CPtr& pBlock, const mw::Hash& canonical_block_hash);
    void BlockDisconnected(const mw::Block::CPtr& pBlock);

private:
    libmw::IWallet::Ptr m_pWalletInterface;
    SecretKey m_masterKey;
    PublicKey m_masterPub;

    Output CreateOutput(
        const uint64_t amount,
        const EOutputFeatures features,
        const libmw::PrivateKey& private_key
    ) const;

    SecretKey RewindNonce(const Commitment& commitment) const;
    SecretKey SecretNonce(const Commitment& commitment) const;
};