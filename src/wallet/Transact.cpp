#include "Transact.h"
#include "CoinSelection.h"
#include "WalletUtil.h"

#include <mw/consensus/Weight.h>
#include <mw/crypto/Blinds.h>
#include <mw/exceptions/InsufficientFundsException.h>
#include <mw/wallet/Wallet.h>

mw::Transaction::CPtr Transact::CreateTx(
    const uint64_t amount,
    const uint64_t fee_base,
    const StealthAddress& receiver_addr) const
{
    // Calculate fee
    const uint64_t fee = fee_base * Weight::Calculate({ .num_kernels = 1, .num_owner_sigs = 1, .num_outputs = 2 });

    // Select coins
    std::vector<libmw::Coin> input_coins = CoinSelection::SelectCoins(m_wallet.GetInterface(), amount + fee);
    if (WalletUtil::TotalAmount(input_coins) < (amount + fee)) {
        ThrowInsufficientFunds("Not enough funds");
    }

    // Sign inputs
    std::vector<Input> inputs = WalletUtil::SignInputs(input_coins);

    // Create receiver's output
    BlindingFactor receiver_blind;
    SecretKey ephemeral_key = Random::CSPRNG<32>();
    Output receiver_output = Output::Create(
        receiver_blind,
        EOutputFeatures::DEFAULT_OUTPUT,
        ephemeral_key,
        receiver_addr,
        amount
    );

    // Create change output
    const uint64_t change_amount = WalletUtil::TotalAmount(input_coins) - (amount + fee);
    BlindingFactor change_blind;
    SecretKey change_key = Random::CSPRNG<32>();
    Output change_output = Output::Create(
        change_blind,
        EOutputFeatures::DEFAULT_OUTPUT,
        change_key,
        m_wallet.GetChangeAddress(),
        change_amount
    );

    // Total kernel offset is split between raw kernel_offset and the kernel's blinding factor.
    // sum(output.blind) - sum(input.blind) = kernel_offset + sum(kernel.blind)
    std::vector<BlindingFactor> input_blinds = WalletUtil::GetBlindingFactors(input_coins);
    BlindingFactor kernel_offset = Random::CSPRNG<32>();
    BlindingFactor kernel_blind = Blinds()
        .Add(receiver_blind)
        .Add(change_blind)
        .Sub(input_blinds)
        .Sub(kernel_offset)
        .Total();
    Kernel kernel = Kernel::CreatePlain(kernel_blind, fee);

    // FUTURE: Only necessary when no change?
    BlindingFactor owner_sig_key = Random::CSPRNG<32>();
    SignedMessage owner_sig = Schnorr::SignMessage(owner_sig_key.GetBigInt(), kernel.GetHash());

    // Total owner offset is split between raw owner_offset and the owner_sig's key.
    // sum(output.sender_key) - sum(input.key) = owner_offset + sum(owner_sig.key)
    std::vector<BlindingFactor> input_keys = WalletUtil::GetKeys(input_coins);
    BlindingFactor owner_offset = Blinds()
        .Add(ephemeral_key)
        .Add(change_key)
        .Sub(input_keys)
        .Sub(owner_sig_key)
        .Total();

    // Rewind the change output to retrieve the spendable coin.
    // This uses the same process that occurs on restore,
    // so a successful rewind means we know it can be restored from seed later.
    libmw::Coin change_coin = m_wallet.RewindOutput(change_output);

    // Add/update the affected coins in the database.
    std::vector<libmw::Coin> coins_to_update = input_coins;
    coins_to_update.push_back(std::move(change_coin));

    m_wallet.GetInterface()->AddCoins(coins_to_update);

    // Build the transaction
    return mw::Transaction::Create(
        kernel_offset,
        owner_offset,
        inputs,
        std::vector<Output>{ std::move(receiver_output), std::move(change_output) },
        std::vector<Kernel>{ std::move(kernel) },
        std::vector<SignedMessage>{ std::move(owner_sig) }
    );
}