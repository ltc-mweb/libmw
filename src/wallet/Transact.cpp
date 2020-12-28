#include "Transact.h"
#include "CoinSelection.h"
#include "WalletUtil.h"

#include <mw/crypto/Blinds.h>
#include <mw/exceptions/InsufficientFundsException.h>
#include <mw/wallet/Wallet.h>
#include <mw/wallet/KernelFactory.h>
#include <mw/wallet/OutputFactory.h>

mw::Transaction::CPtr Transact::CreateTx(
    const uint64_t amount,
    const uint64_t fee_base,
    const StealthAddress& receiver_addr) const
{
    // Select coins
    std::vector<libmw::Coin> input_coins = CoinSelection::SelectCoins(m_wallet.GetInterface(), amount, fee_base);
    std::vector<Input> inputs = WalletUtil::SignInputs(input_coins);

    // Calculate fee
    const uint64_t fee = WalletUtil::CalculateFee(fee_base, input_coins.size(), 1, 2);
    if (WalletUtil::TotalAmount(input_coins) <= (amount + fee)) {
        ThrowInsufficientFunds("Not enough funds");
    }

    // Create receiver's output
    BlindingFactor receiver_blind = Random().CSPRNG<32>();
    SecretKey ephemeral_key = m_wallet.NewKey();
    Output receiver_output = OutputFactory::Create(
        EOutputFeatures::DEFAULT_OUTPUT,
        receiver_blind,
        ephemeral_key,
        receiver_addr,
        amount
    );

    // Create change output
    const uint64_t change_amount = WalletUtil::TotalAmount(input_coins) - (amount + fee);
    BlindingFactor change_blind = Random().CSPRNG<32>();
    SecretKey change_key = m_wallet.NewKey();
    Output change_output = OutputFactory::Create(
        EOutputFeatures::DEFAULT_OUTPUT,
        change_blind,
        change_key,
        m_wallet.GetStealthAddress(),
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
    Kernel kernel = KernelFactory::CreatePlainKernel(kernel_blind, fee);

    // TODO: Only necessary when no change?
    BlindingFactor owner_sig_key = Random().CSPRNG<32>();
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
    return std::make_shared<mw::Transaction>(
        std::move(kernel_offset),
        std::move(owner_offset),
        TxBody{
            std::move(inputs),
            std::vector<Output>{ std::move(receiver_output), std::move(change_output) },
            std::vector<Kernel>{ std::move(kernel) },
            std::vector<SignedMessage>{ std::move(owner_sig) }
        }
    );
}