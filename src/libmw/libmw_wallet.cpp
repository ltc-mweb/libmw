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

MWEXPORT bool RewindOutput(
    const libmw::IWallet::Ptr& pWallet,
    const libmw::TxRef& tx,
    const libmw::Commitment& output_commit,
    libmw::Coin& coin_out)
{
    assert(tx.pTransaction != nullptr);

    ::Commitment commitment(output_commit);
    for (const Output& output : tx.pTransaction->GetOutputs()) {
        if (output.GetCommitment() == commitment) {
            try {
                coin_out = Wallet::Open(pWallet).RewindOutput(output);
                return true;
            }
            catch (...) {}

            break;
        }
    }

    return false;
}

END_NAMESPACE // wallet
END_NAMESPACE // libmw