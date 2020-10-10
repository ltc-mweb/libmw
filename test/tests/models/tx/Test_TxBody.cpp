#include <catch.hpp>

#include <mw/crypto/Crypto.h>
#include <mw/crypto/Random.h>
#include <mw/models/tx/TxBody.h>

#include <test_framework/models/TxOutput.h>

TEST_CASE("Tx Body")
{
    const uint64_t pegInAmount = 123;

    BlindingFactor blind_a = Random::CSPRNG<32>().GetBigInt();
    BlindingFactor blind_b = Random::CSPRNG<32>().GetBigInt();
    BlindingFactor blind_c = Random::CSPRNG<32>().GetBigInt();
    BlindingFactor blind_d = Random::CSPRNG<32>().GetBigInt();

    std::vector<Input> inputs{
        Input(EOutputFeatures::DEFAULT_OUTPUT, Crypto::CommitBlinded(20, blind_a)),
        Input(EOutputFeatures::PEGGED_IN, Crypto::CommitBlinded(30, blind_b))
    };
    std::vector<Output> outputs{
        test::TxOutput::Create(EOutputFeatures::DEFAULT_OUTPUT, blind_c, 45).GetOutput(),
        test::TxOutput::Create(EOutputFeatures::PEGGED_IN, blind_d, pegInAmount).GetOutput()
    };

    std::vector<Kernel> kernels;
    // For simplification no offsets are used.
    {
        BlindingFactor kernelBF = Crypto::AddBlindingFactors({ blind_c }, { blind_a, blind_b });
        Commitment kernelCommit = Crypto::CommitBlinded(0, kernelBF);

        Serializer serializer;
        serializer.Append<uint8_t>((uint8_t)KernelType::PLAIN_KERNEL);
        //TODO: anything else to append?

        Signature signature = Crypto::BuildSignature(
            kernelBF.ToSecretKey(),
            Hashed(serializer.vec())
        );

        kernels.push_back(Kernel::CreatePlain(5, std::move(kernelCommit), std::move(signature)));
    }
    {
        BlindingFactor kernelBF = Crypto::AddBlindingFactors({ blind_d }, { });
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

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = txBody.Serialized();
        REQUIRE(serialized.size() == 1740);

        Deserializer deserializer(serialized);
        REQUIRE(deserializer.Read<uint64_t>() == inputs.size());
        REQUIRE(deserializer.Read<uint64_t>() == outputs.size());
        REQUIRE(deserializer.Read<uint64_t>() == kernels.size());

        Deserializer deserializer2(serialized);
        REQUIRE(txBody == TxBody::Deserialize(deserializer2));
    }

    //
    // JSON
    //
    {
        Json json(txBody.ToJSON());
        REQUIRE(json.GetKeys() == std::vector<std::string>({ "inputs", "kernels", "outputs" }));
        REQUIRE(json.GetRequiredVec<Input>("inputs") == inputs);
        REQUIRE(json.GetRequiredVec<Output>("outputs") == outputs);
        REQUIRE(json.GetRequiredVec<Kernel>("kernels") == kernels);

        REQUIRE(txBody == TxBody::FromJSON(json));
    }

    //
    // Getters
    //
    {
        REQUIRE(txBody.GetInputs() == inputs);
        REQUIRE(txBody.GetOutputs() == outputs);
        REQUIRE(txBody.GetKernels() == kernels);
        REQUIRE(txBody.GetTotalFee() == 5);
    }
}