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
            pegouts.push_back(PegOutCoin(pegout_recipient.amount, pegout_recipient.scriptPubKey));
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

MWEXPORT libmw::MWEBAddress GetAddress(const libmw::IWallet::Ptr& pWallet, const uint32_t index)
{
    return Wallet::Open(pWallet).GetStealthAddress(index).Encode();
}

MWEXPORT bool RewindOutput(
    const libmw::IWallet::Ptr& pWallet,
    const boost::variant<libmw::TxRef, libmw::BlockRef>& parent,
    const libmw::Commitment& output_commit,
    libmw::Coin& coin_out)
{
    ::Commitment commitment(output_commit);

    if (parent.type() == typeid(libmw::TxRef)) {
        const libmw::TxRef& tx = boost::get<libmw::TxRef>(parent);
        assert(tx.pTransaction != nullptr);

        for (const Output& output : tx.pTransaction->GetOutputs()) {
            if (output.GetCommitment() == commitment) {
                return Wallet::Open(pWallet).RewindOutput(output, coin_out);
            }
        }
    } else {
        const libmw::BlockRef& block = boost::get<libmw::BlockRef>(parent);
        assert(block.pBlock != nullptr);

        for (const Output& output : block.pBlock->GetOutputs()) {
            if (output.GetCommitment() == commitment) {
                return Wallet::Open(pWallet).RewindOutput(output, coin_out);
            }
        }
    }

    return false;
}

END_NAMESPACE // wallet
END_NAMESPACE // libmw