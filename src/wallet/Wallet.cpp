#include <mw/wallet/Wallet.h>
#include <mw/crypto/Random.h>

mw::Transaction::CPtr Wallet::CreatePegInTx(const uint64_t amount)
{
    libmw::PrivateKey private_key = m_pWalletInterface->GenerateNewHDKey();
    BlindingFactor blindingFactor(BigInt<32>{ private_key.keyBytes });
    BlindingFactor offset = Random::CSPRNG<32>();

    Output output = CreateOutput(amount, EOutputFeatures::PEGGED_IN,  blindingFactor);

    BlindingFactor kernel_blind = Crypto::AddBlindingFactors({ blindingFactor }, { offset });
    Commitment kernel_commit = Crypto::CommitBlinded(0, kernel_blind);
    mw::Hash sig_message = Hashed(Serializer().Append<uint8_t>(KernelType::PEGIN_KERNEL).Append<uint64_t>(amount).vec());
    Signature sig = Crypto::BuildSignature(kernel_blind.ToSecretKey(), sig_message);
    Kernel kernel = Kernel::CreatePegIn(amount, std::move(kernel_commit), std::move(sig));

    m_pWalletInterface->AddCoins({ libmw::Coin{ private_key, amount } });

    TxBody body(
        std::vector<Input>{},
        std::vector<Output>{ std::move(output) },
        std::vector<Kernel>{ std::move(kernel) }
    );
    return std::make_shared<mw::Transaction>(std::move(offset), std::move(body));
}

mw::Transaction::CPtr Wallet::CreatePegOutTx(const uint64_t amount, const Bech32Address& address)
{
    libmw::PrivateKey private_key = m_pWalletInterface->GenerateNewHDKey();
    BlindingFactor blindingFactor(BigInt<32>{ private_key.keyBytes });
    BlindingFactor offset = Random::CSPRNG<32>();

    std::vector<libmw::Coin> coins = m_pWalletInterface->ListCoins();
    LOG_INFO_F("Num Coins: {}", coins.size());

    std::vector<libmw::Coin> inputs;

    const uint64_t fee = 1'000'000;
    uint64_t inputs_amount = 0;
    std::vector<BlindingFactor> blinding_factors;
    for (const libmw::Coin& coin : coins) {
        inputs_amount += coin.amount;
        inputs.push_back(coin);
        blinding_factors.push_back(BlindingFactor(BigInt<32>(coin.key.keyBytes)));

        if (inputs_amount >= amount + fee) {
            break;
        }
    }

    if (inputs_amount <= amount + fee) {
        throw std::exception(); // TODO: Throw InsufficientFundsException
    }

    const uint64_t change_amount = inputs_amount - (amount + fee);
    Output change_output = CreateOutput(change_amount, EOutputFeatures::DEFAULT_OUTPUT, blindingFactor);

    blinding_factors.push_back(offset);
    BlindingFactor kernel_blind = Crypto::AddBlindingFactors({ blindingFactor }, blinding_factors);
    Commitment kernel_commit = Crypto::CommitBlinded(0, kernel_blind);
    mw::Hash sig_message = Hashed(Serializer().Append<uint8_t>(KernelType::PEGOUT_KERNEL).Append<uint64_t>(fee).Append<uint64_t>(amount).Append(address).vec());
    Signature sig = Crypto::BuildSignature(kernel_blind.ToSecretKey(), sig_message);
    Kernel kernel = Kernel::CreatePegOut(amount, fee, Bech32Address(address), std::move(kernel_commit), std::move(sig));

    m_pWalletInterface->DeleteCoins(inputs);
    m_pWalletInterface->AddCoins({ libmw::Coin{ libmw::PrivateKey{ "", blindingFactor.array() }, change_amount, change_output.GetCommitment().array() } });

    TxBody body(
        std::vector<Input>{},
        std::vector<Output>{ std::move(change_output) },
        std::vector<Kernel>{ std::move(kernel) }
    );
    return std::make_shared<mw::Transaction>(std::move(offset), std::move(body));
}

Output Wallet::CreateOutput(const uint64_t amount, const EOutputFeatures features, const BlindingFactor& blindingFactor) const
{
    Commitment commitment = Crypto::CommitBlinded(amount, blindingFactor);
    RangeProof::CPtr pRangeProof = CreateRangeProof(amount, commitment, blindingFactor);

    return Output{ features, std::move(commitment), {}, pRangeProof };
}

RangeProof::CPtr Wallet::CreateRangeProof(
    const uint64_t amount,
    const Commitment& commitment,
    const BlindingFactor& blindingFactor) const
{
    BlindingFactor blind(blindingFactor);
    return Crypto::GenerateRangeProof(amount, blind.ToSecretKey(), SecretKey{}, SecretKey{}, ProofMessage({}));
}