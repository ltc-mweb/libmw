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
    Kernel kernel = Kernel::CreatePegIn(kernelBF, amount);

    auto pTx = mw::Transaction::Create(txOffset, sender_privkey, {}, { output.GetOutput() }, { kernel }, {});
    return Tx{ pTx, { output } };
}