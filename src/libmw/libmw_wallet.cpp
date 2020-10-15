#include <libmw/wallet.h>

#include "Transformers.h"

#include <mw/models/tx/Transaction.h>
#include <mw/wallet/Wallet.h>

LIBMW_NAMESPACE
WALLET_NAMESPACE

MWEXPORT std::pair<libmw::TxRef, libmw::PegIn> CreatePegInTx(const libmw::IWallet::Ptr& pWallet, const uint64_t amount)
{
    mw::Transaction::CPtr pTx = Wallet(pWallet).CreatePegInTx(amount);

    assert(!pTx->GetKernels().empty());
    libmw::Commitment commit = pTx->GetKernels().front().GetCommitment().array();
    return std::make_pair(libmw::TxRef{ pTx }, libmw::PegIn{ amount, commit });
}

MWEXPORT std::pair<libmw::TxRef, libmw::PegOut> CreatePegOutTx(
    const libmw::IWallet::Ptr& pWallet,
    const uint64_t amount,
    const std::string& address)
{
    mw::Transaction::CPtr pTx = Wallet(pWallet).CreatePegOutTx(amount, Bech32Address::FromString(address));
    return std::make_pair(libmw::TxRef{ pTx }, libmw::PegOut{ amount, address });
}

END_NAMESPACE // wallet
END_NAMESPACE // libmw