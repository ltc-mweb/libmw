#include <catch.hpp>

#include <mw/crypto/Crypto.h>
#include <mw/crypto/Random.h>
#include <mw/models/tx/Transaction.h>
#include <mw/consensus/BlockSumValidator.h>

#include <test_framework/models/TxOutput.h>

TEST_CASE("Tx Transaction")
{
    const uint64_t pegInAmount = 123;
    const uint64_t fee = 5;

    BlindingFactor blind_a = Random::CSPRNG<32>().GetBigInt();
    BlindingFactor blind_b = Random::CSPRNG<32>().GetBigInt();
    BlindingFactor blind_c = Random::CSPRNG<32>().GetBigInt();
    BlindingFactor blind_d = Random::CSPRNG<32>().GetBigInt();

    BlindingFactor offset_a = Random::CSPRNG<32>().GetBigInt();
    BlindingFactor offset_b = Random::CSPRNG<32>().GetBigInt();
    BlindingFactor totalOffset = Crypto::AddBlindingFactors({ offset_a, offset_b }, { });

    std::vector<Input> inputs{
        Input(EOutputFeatures::DEFAULT_OUTPUT, Crypto::CommitBlinded(20, blind_a)),
        Input(EOutputFeatures::PEGGED_IN, Crypto::CommitBlinded(30, blind_b))
    };
    std::vector<Output> outputs{
        test::TxOutput::Create(EOutputFeatures::DEFAULT_OUTPUT, blind_c, 45).GetOutput(),
        test::TxOutput::Create(EOutputFeatures::PEGGED_IN, blind_d, pegInAmount).GetOutput()
    };

    std::vector<Kernel> kernels;
    {
        BlindingFactor kernelBF = Crypto::AddBlindingFactors({ blind_c }, { blind_a, blind_b, offset_a });
        Commitment kernelCommit = Crypto::CommitBlinded(0, kernelBF);

        Serializer serializer;
        serializer.Append<uint8_t>((uint8_t)KernelType::PLAIN_KERNEL);
        serializer.Append<uint64_t>(fee);

        Signature signature = Crypto::BuildSignature(
            kernelBF.ToSecretKey(),
            Hashed(serializer.vec())
        );

        kernels.push_back(Kernel::CreatePlain(fee, std::move(kernelCommit), std::move(signature)));
    }
    {
        BlindingFactor kernelBF = Crypto::AddBlindingFactors({ blind_d }, { offset_b });
        Commitment kernelCommit = Crypto::CommitBlinded(0, kernelBF);

        Serializer serializer;
        serializer.Append<uint8_t>((uint8_t)KernelType::PEGIN_KERNEL);
        serializer.Append<uint64_t>(pegInAmount);

        Signature signature = Crypto::BuildSignature(
            kernelBF.ToSecretKey(),
            Hashed(serializer.vec())
        );

        kernels.push_back(Kernel::CreatePegIn(pegInAmount, std::move(kernelCommit), std::move(signature)));
    }

    TxBody txBody{
        std::vector<Input>(inputs),
        std::vector<Output>(outputs),
        std::vector<Kernel>(kernels)
    };
    txBody.Validate();

    mw::Transaction tx{
        BlindingFactor(totalOffset),
        TxBody(txBody)
    };
    BlockSumValidator::ValidateForTx(tx);

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = tx.Serialized();
        REQUIRE(serialized.size() == 1772);

        Deserializer deserializer(serialized);
        REQUIRE(BlindingFactor::Deserialize(deserializer) == totalOffset);
        REQUIRE(TxBody::Deserialize(deserializer) == txBody);

        Deserializer deserializer2(serialized);
        REQUIRE(tx == mw::Transaction::Deserialize(deserializer2));
    }

    //
    // JSON
    //
    {
        Json json(tx.ToJSON());
        REQUIRE(json.GetKeys() == std::vector<std::string>({ "body", "offset" }));
        REQUIRE(BlindingFactor::FromHex(json.GetRequired<std::string>("offset")) == totalOffset);
        REQUIRE(json.GetRequired<TxBody>("body") == txBody);

        REQUIRE(tx == mw::Transaction::FromJSON(json));
    }

    //
    // Getters
    //
    {
        REQUIRE(tx.GetOffset() == totalOffset);
        REQUIRE(tx.GetBody() == txBody);
        REQUIRE(tx.GetInputs() == inputs);
        REQUIRE(tx.GetOutputs() == outputs);
        REQUIRE(tx.GetKernels() == kernels);
        REQUIRE(tx.GetTotalFee() == fee);
    }
}