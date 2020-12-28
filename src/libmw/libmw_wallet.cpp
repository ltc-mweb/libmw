#include <libmw/wallet.h>

#include "Transformers.h"

#include <mw/models/tx/Transaction.h>
#include <mw/wallet/Wallet.h>

LIBMW_NAMESPACE
WALLET_NAMESPACE

// TODO: Support passing in receiver_addr
MWEXPORT std::pair<libmw::TxRef, libmw::PegIn> CreatePegInTx(const libmw::IWallet::Ptr& pWallet, const uint64_t amount)
{
    mw::Transaction::CPtr pTx = Wallet::Open(pWallet).CreatePegInTx(amount, boost::none);

    assert(!pTx->GetKernels().empty());
    libmw::Commitment commit = pTx->GetKernels().front().GetCommitment().array();
    return std::make_pair(libmw::TxRef{ pTx }, libmw::PegIn{ amount, commit });
}

MWEXPORT std::pair<libmw::TxRef, libmw::PegOut> CreatePegOutTx(
    const libmw::IWallet::Ptr& pWallet,
    const uint64_t amount,
    const uint64_t fee_base,
    const std::string& address)
{
    mw::Transaction::CPtr pTx = Wallet::Open(pWallet).CreatePegOutTx(
        amount,
        fee_base,
        Bech32Address::FromString(address)
    );
    return std::make_pair(libmw::TxRef{ pTx }, libmw::PegOut{ amount, address });
}

MWEXPORT libmw::TxRef Send(
    const libmw::IWallet::Ptr& pWallet,
    const uint64_t amount,
    const uint64_t fee_base,
    const libmw::MWEBAddress& address)
{
    auto pTx = Wallet::Open(pWallet).Send(amount, fee_base, StealthAddress::Decode(address));
    return libmw::TxRef{ pTx };
}

MWEXPORT void BlockConnected(
    const libmw::IWallet::Ptr& pWallet,
    const libmw::BlockRef& block,
    const libmw::BlockHash& canonical_block_hash)
{
    Wallet::Open(pWallet).BlockConnected(block.pBlock, canonical_block_hash);
}

MWEXPORT void BlockDisconnected(
    const libmw::IWallet::Ptr& pWallet,
    const libmw::BlockRef& block)
{
    Wallet::Open(pWallet).BlockDisconnected(block.pBlock);
}

MWEXPORT void ScanForOutputs(const libmw::IWallet::Ptr& pWallet, const libmw::IChain::Ptr& pChain)
{
    Wallet::Open(pWallet).ScanForOutputs(pChain);
}

MWEXPORT libmw::MWEBAddress GetAddress(const libmw::IWallet::Ptr& pWallet)
{
    return Wallet::Open(pWallet).GetStealthAddress().Encode();
}

MWEXPORT libmw::WalletBalance GetBalance(const libmw::IWallet::Ptr& pWallet)
{
    return Wallet::Open(pWallet).GetBalance();
}

END_NAMESPACE // wallet
END_NAMESPACE // libmw