#include <catch.hpp>

#include <test_framework/TxBuilder.h>

TEST_CASE("Tx Body")
{
    const uint64_t pegInAmount = 123;
    const uint64_t fee = 5;

    mw::Transaction::CPtr tx = test::TxBuilder()
        .AddInput(20).AddInput(30, EOutputFeatures::PEGGED_IN)
        .AddOutput(45).AddOutput(pegInAmount, EOutputFeatures::PEGGED_IN)
        .AddPlainKernel(fee).AddPeginKernel(pegInAmount)
        .Build();

    const TxBody& txBody = tx->GetBody();
    txBody.Validate();

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = txBody.Serialized();
        REQUIRE(serialized.size() == 2066);

        Deserializer deserializer(serialized);
        REQUIRE(deserializer.Read<uint64_t>() == txBody.GetInputs().size());
        REQUIRE(deserializer.Read<uint64_t>() == txBody.GetOutputs().size());
        REQUIRE(deserializer.Read<uint64_t>() == txBody.GetKernels().size());

        Deserializer deserializer2(serialized);
        REQUIRE(txBody == TxBody::Deserialize(deserializer2));
    }

    //
    // JSON
    //
    {
        Json json(txBody.ToJSON());
        REQUIRE(json.GetKeys() == std::vector<std::string>({ "inputs", "kernels", "outputs" }));
        REQUIRE(json.GetRequiredVec<Input>("inputs") == txBody.GetInputs());
        REQUIRE(json.GetRequiredVec<Output>("outputs") == txBody.GetOutputs());
        REQUIRE(json.GetRequiredVec<Kernel>("kernels") == txBody.GetKernels());

        REQUIRE(txBody == TxBody::FromJSON(json));
    }

    //
    // Getters
    //
    {
        REQUIRE(txBody.GetTotalFee() == fee);
    }
}