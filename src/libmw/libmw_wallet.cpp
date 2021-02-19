#include <libmw/wallet.h>

#include "Transformers.h"

#include <mw/models/tx/Transaction.h>
#include <mw/wallet/Wallet.h>

LIBMW_NAMESPACE
WALLET_NAMESPACE

MWEXPORT libmw::TxRef CreateTx(
    const libmw::IWallet::Ptr& pWallet,
    const std::vector<libmw::Commitment>& selected_inputs,
    const std::vector<libmw::Recipient>& recipients,
    const boost::optional<uint64_t>& pegin_amount,
    const uint64_t fee)
{
    std::vector<std::pair<uint64_t, StealthAddress>> receivers;
    std::vector<PegOutCoin> pegouts;

    for (const libmw::Recipient& recipient : recipients) {
        if (recipient.which() == 0) {
            const libmw::MWEBRecipient& mweb_recipient = boost::get<libmw::MWEBRecipient>(recipient);
            receivers.push_back(std::make_pair(mweb_recipient.amount, StealthAddress::Decode(mweb_recipient.address)));
        } else if (recipient.which() == 1) {
            const libmw::PegInRecipient& pegin_recipient = boost::get<libmw::PegInRecipient>(recipient);
            receivers.push_back(std::make_pair(pegin_recipient.amount, StealthAddress::Decode(pegin_recipient.address)));
        } else {
            const libmw::PegOutRecipient& pegout_recipient = boost::get<libmw::PegOutRecipient>(recipient);
            pegouts.push_back(PegOutCoin(pegout_recipient.amount, Bech32Address::FromString(pegout_recipient.address)));
        }
    }

    mw::Transaction::CPtr pTransaction = Wallet::Open(pWallet).CreateTx(
        TransformCommitments(selected_inputs),
        receivers,
        pegouts,
        pegin_amount,
        fee
    );
    return libmw::TxRef{ pTransaction };
}

MWEXPORT std::pair<libmw::TxRef, libmw::PegIn> CreatePegInTx(
    const libmw::IWallet::Ptr& pWallet,
    const uint64_t amount,
    const libmw::MWEBAddress& address)
{
    boost::optional<StealthAddress> pegin_address = boost::none;
    if (!address.empty()) {
        pegin_address = boost::make_optional(StealthAddress::Decode(address));
    }

    mw::Transaction::CPtr pTx = Wallet::Open(pWallet).CreatePegInTx(amount, pegin_address);

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

MWEXPORT void TransactionAddedToMempool(
    const libmw::IWallet::Ptr& pWallet,
    const libmw::TxRef& tx)
{
    Wallet::Open(pWallet).TransactionAddedToMempool(tx.pTransaction);
}

MWEXPORT void ScanForOutputs(const libmw::IWallet::Ptr& pWallet, const libmw::IChain::Ptr& pChain)
{
    Wallet::Open(pWallet).ScanForOutputs(pChain);
}

MWEXPORT libmw::MWEBAddress GetAddress(const libmw::IWallet::Ptr& pWallet, const uint32_t index)
{
    return Wallet::Open(pWallet).GetStealthAddress(index).Encode();
}

MWEXPORT bool IsOwnAddress(const libmw::IWallet::Ptr& pWallet, const libmw::MWEBAddress& address)
{
    return GetAddress(pWallet, 0) == address; // TODO: Check all addresses
}

MWEXPORT libmw::WalletBalance GetBalance(const libmw::IWallet::Ptr& pWallet)
{
    return Wallet::Open(pWallet).GetBalance();
}

END_NAMESPACE // wallet
END_NAMESPACE // libmw