#include <test_framework/models/Tx.h>

test::Tx test::Tx::CreatePegIn(const uint64_t amount)
{
    BlindingFactor txOffset = Random::CSPRNG<32>();

    BlindingFactor outputBF = Random::CSPRNG<32>();
    SecretKey sender_privkey = Random::CSPRNG<32>();
    test::TxOutput output = test::TxOutput::Create(
        EOutputFeatures::PEGGED_IN,
        outputBF,
        sender_privkey,
        StealthAddress::Random(),
        amount
    );

    BlindingFactor kernelBF = Crypto::AddBlindingFactors({ outputBF }, { txOffset });
    Commitment kernelCommit = Crypto::CommitBlinded(0, kernelBF);

    Signature signature = Schnorr::Sign(
        kernelBF.data(),
        Hasher()
            .Append<uint8_t>((uint8_t)KernelType::PEGIN_KERNEL)
            .Append<uint64_t>(amount)
            .hash()
    );

    Kernel kernel = Kernel::CreatePegIn(amount, std::move(kernelCommit), std::move(signature));

    auto pTx = mw::Transaction::Create(txOffset, sender_privkey, {}, { output.GetOutput() }, { kernel }, {});
    return Tx{ pTx, { output } };
}