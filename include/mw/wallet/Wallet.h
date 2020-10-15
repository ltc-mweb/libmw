#pragma once

#include <libmw/interfaces/wallet_interface.h>
#include <mw/models/tx/Transaction.h>
#include <mw/models/crypto/Bech32Address.h>

class Wallet
{
public:
    Wallet(const libmw::IWallet::Ptr& pWalletInterface)
        : m_pWalletInterface(pWalletInterface) { }
    
    mw::Transaction::CPtr CreatePegInTx(const uint64_t amount);
    mw::Transaction::CPtr CreatePegOutTx(const uint64_t amount, const Bech32Address& address);

private:
    libmw::IWallet::Ptr m_pWalletInterface;

    Output CreateOutput(const uint64_t amount, const EOutputFeatures features, const BlindingFactor& blindingFactor) const;

    RangeProof::CPtr CreateRangeProof(
        const uint64_t amount,
        const Commitment& commitment,
        const BlindingFactor& blindingFactor
    ) const;
};