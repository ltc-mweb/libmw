#include "Transact.h"
#include "WalletUtil.h"

#include <mw/consensus/Weight.h>
#include <mw/crypto/Blinds.h>
#include <mw/exceptions/InsufficientFundsException.h>
#include <mw/wallet/Wallet.h>

mw::Transaction::CPtr Transact::CreateTx(
    const std::vector<Commitment>& input_commits,
    const std::vector<std::pair<uint64_t, StealthAddress>>& recipients,
    const std::vector<PegOutCoin>& pegouts,
    const boost::optional<uint64_t>& pegin_amount,
    const uint64_t fee) const
{
    uint64_t pegout_total = std::accumulate(
        pegouts.cbegin(), pegouts.cend(), (uint64_t)0,
        [](uint64_t sum, const PegOutCoin& pegout) { return sum + pegout.GetAmount(); }
    );

    uint64_t recipient_total = std::accumulate(
        recipients.cbegin(), recipients.cend(), (uint64_t)0,
        [](uint64_t sum, const std::pair<uint64_t, StealthAddress>& recipient) { return sum + recipient.first; }
    );

    // Get input coins
    std::vector<libmw::Coin> input_coins = m_wallet.GetCoins(input_commits);
    if ((WalletUtil::TotalAmount(input_coins) + pegin_amount.value_or(0)) == (pegout_total + recipient_total + fee)) {
        // TODO: ThrowInsufficientFunds("Total amount mismatch");
    }

    // Sign inputs
    std::vector<Input> inputs = WalletUtil::SignInputs(input_coins);

    // Create outputs
    Transact::Outputs outputs = CreateOutputs(recipients);

    // Total kernel offset is split between raw kernel_offset and the kernel's blinding factor.
    // sum(output.blind) - sum(input.blind) = kernel_offset + sum(kernel.blind)
    std::vector<BlindingFactor> input_blinds = WalletUtil::GetBlindingFactors(input_coins);
    BlindingFactor kernel_offset = Random::CSPRNG<32>();
    BlindingFactor kernel_blind = Blinds()
        .Add(outputs.total_blind)
        .Sub(input_blinds)
        .Sub(kernel_offset)
        .Total();

    Kernel kernel = Kernel::CreatePlain(kernel_blind, fee);
    if (pegin_amount) {
        kernel = Kernel::CreatePegIn(kernel_blind, *pegin_amount + fee); // TODO: Add fee field to peg-in kernel.
    }

    // FUTURE: Only necessary when none of the addresses are owned by this wallet?
    BlindingFactor owner_sig_key = Random::CSPRNG<32>();
    SignedMessage owner_sig = Schnorr::SignMessage(owner_sig_key.GetBigInt(), kernel.GetHash());

    // Total owner offset is split between raw owner_offset and the owner_sig's key.
    // sum(output.sender_key) - sum(input.key) = owner_offset + sum(owner_sig.key)
    std::vector<BlindingFactor> input_keys = WalletUtil::GetKeys(input_coins);
    BlindingFactor owner_offset = Blinds()
        .Add(outputs.total_key)
        .Sub(input_keys)
        .Sub(owner_sig_key)
        .Total();

    // Add/update the affected coins in the database.
    {
        std::vector<libmw::Coin> coins_to_update = input_coins;

        // Rewind the output to retrieve the spendable coins in cases where we own the receiving address (i.e. change).
        // This uses the same process that occurs on restore,
        // so a successful rewind means we know it can be restored from seed later.
        for (const Output& output : outputs.outputs) {
            try {
                libmw::Coin coin = m_wallet.RewindOutput(output);
                coins_to_update.push_back(std::move(coin));
            } catch (const std::exception&) { }
        }

        m_wallet.GetInterface()->AddCoins(coins_to_update);
    }

    // Build the transaction
    return mw::Transaction::Create(
        kernel_offset,
        owner_offset,
        inputs,
        std::move(outputs.outputs),
        std::vector<Kernel>{ std::move(kernel) },
        std::vector<SignedMessage>{ std::move(owner_sig) }
    );
}

Transact::Outputs Transact::CreateOutputs(const std::vector<std::pair<uint64_t, StealthAddress>>& recipients) const
{
    Blinds output_blinds;
    Blinds output_keys;
    std::vector<Output> outputs;
    std::transform(
        recipients.cbegin(), recipients.cend(),
        std::back_inserter(outputs),
        [&output_blinds, &output_keys](const std::pair<uint64_t, StealthAddress>& recipient) {
            BlindingFactor blind;
            SecretKey ephemeral_key = Random::CSPRNG<32>();
            Output output = Output::Create(
                blind,
                EOutputFeatures::DEFAULT_OUTPUT,
                ephemeral_key,
                recipient.second,
                recipient.first
            );

            output_blinds.Add(blind);
            output_keys.Add(ephemeral_key);
            return output;
        }
    );

    return Transact::Outputs{
        output_blinds.Total(),
        output_keys.Total(),
        std::move(outputs)
    };
}