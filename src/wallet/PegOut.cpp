#include "PegOut.h"
#include "CoinSelection.h"
#include "WalletUtil.h"

#include <mw/consensus/Weight.h>
#include <mw/crypto/Blinds.h>
#include <mw/exceptions/InsufficientFundsException.h>
#include <mw/wallet/Wallet.h>
#include <mw/wallet/KernelFactory.h>
#include <mw/wallet/OutputFactory.h>
#include <mw/wallet/TxFactory.h>

mw::Transaction::CPtr PegOut::CreatePegOutTx(
    const uint64_t amount,
    const uint64_t fee_base,
    const Bech32Address& address) const
{
    // Calculate fee
    const uint64_t fee = fee_base * Weight::Calculate({ .num_kernels = 1, .num_owner_sigs = 1, .num_outputs = 2 });

    // Select coins
    std::vector<libmw::Coin> input_coins = CoinSelection::SelectCoins(m_wallet.GetInterface(), amount + fee, fee_base);
    if (WalletUtil::TotalAmount(input_coins) < (amount + fee)) {
        ThrowInsufficientFunds("Not enough funds");
    }

    // Sign inputs
    std::vector<Input> inputs = WalletUtil::SignInputs(input_coins);

    // Create change output.
    // We randomly generate the sender_key and output_blind.
    // Receiver key is generate by OutputFactory using the wallet's stealth address.
    const uint64_t change_amount = WalletUtil::TotalAmount(input_coins) - (amount + fee);
    SecretKey change_key = m_wallet.NewKey();
    BlindingFactor change_blind = Random::CSPRNG<32>();
    Output change_output = OutputFactory::Create(
        EOutputFeatures::DEFAULT_OUTPUT,
        change_blind,
        change_key,
        m_wallet.GetStealthAddress(),
        change_amount
    );

    // Total kernel offset is split between raw kernel_offset and the kernel's blinding factor.
    // sum(output.blind) - sum(input.blind) = kernel_offset + sum(kernel.blind)
    BlindingFactor kernel_offset = Random::CSPRNG<32>();
    std::vector<BlindingFactor> input_blinds = WalletUtil::GetBlindingFactors(input_coins);
    BlindingFactor kernel_blind = Blinds()
        .Add(change_blind)
        .Sub(input_blinds)
        .Sub(kernel_offset)
        .Total();
    Kernel kernel = KernelFactory::CreatePegOutKernel(kernel_blind, amount, fee, address);

    // TODO: Only necessary when no change?
    BlindingFactor owner_sig_key = Random::CSPRNG<32>();
    SignedMessage owner_sig = Schnorr::SignMessage(owner_sig_key.GetBigInt(), kernel.GetHash());

    // Total owner offset is split between raw owner_offset and the owner_sig's key.
    // sum(output.sender_key) - sum(input.key) = owner_offset + sum(owner_sig.key)
    std::vector<BlindingFactor> input_keys = WalletUtil::GetKeys(input_coins);
    BlindingFactor owner_offset = Blinds()
        .Add(change_key)
        .Sub(input_keys)
        .Sub(owner_sig_key)
        .Total();

    // Rewind the change output to retrieve the spendable coin.
    // This uses the same process that occurs on restore,
    // so a successful rewind means we know it can be restored from seed later.
    libmw::Coin change_coin = m_wallet.RewindOutput(change_output);

    // Add/update the affected coins in the database.
    std::vector<libmw::Coin> coins = std::move(input_coins);
    coins.push_back(std::move(change_coin));
    m_wallet.GetInterface()->AddCoins(coins);

    // Build the Transaction
    return TxFactory::CreateTx(
        kernel_offset,
        owner_offset,
        inputs,
        std::vector<Output>{ std::move(change_output) },
        std::vector<Kernel>{ std::move(kernel) },
        std::vector<SignedMessage>{ std::move(owner_sig) }
    );
}