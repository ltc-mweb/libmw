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
        .Build().GetTransaction();

    const TxBody& txBody = tx->GetBody();
    txBody.Validate();

    //
    // Serialization
    //
    {
        std::vector<uint8_t> serialized = txBody.Serialized();

        Deserializer deserializer(serialized);
        REQUIRE(deserializer.Read<uint32_t>() == txBody.GetInputs().size());
        REQUIRE(deserializer.Read<uint32_t>() == txBody.GetOutputs().size());
        REQUIRE(deserializer.Read<uint32_t>() == txBody.GetKernels().size());
        REQUIRE(deserializer.Read<uint32_t>() == txBody.GetOwnerSigs().size());

        Deserializer deserializer2(serialized);
        REQUIRE(txBody == TxBody::Deserialize(deserializer2));
    }

    //
    // Getters
    //
    {
        REQUIRE(txBody.GetTotalFee() == fee);
    }
}