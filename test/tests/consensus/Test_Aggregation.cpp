#include <catch.hpp>

#include <mw/consensus/Aggregation.h>
#include <mw/consensus/BlockSumValidator.h>

#include <test_framework/TxBuilder.h>

TEST_CASE("Aggregation")
{
    mw::Transaction::CPtr tx1 = test::TxBuilder()
        .AddInput(10).AddInput(20)
        .AddOutput(25).AddPlainKernel(5)
        .Build();
    BlockSumValidator::ValidateForTx(*tx1);

    mw::Transaction::CPtr tx2 = test::TxBuilder()
        .AddInput(20)
        .AddOutput(15).AddPlainKernel(5)
        .Build();
    BlockSumValidator::ValidateForTx(*tx2);

    mw::Transaction::CPtr pAggregated = Aggregation::Aggregate({ tx1, tx2 });
    BlockSumValidator::ValidateForTx(*pAggregated);

    std::vector<Input> inputs = tx1->GetInputs();
    inputs.insert(inputs.end(), tx2->GetInputs().begin(), tx2->GetInputs().end());
    std::sort(inputs.begin(), inputs.end(), SortByHash);
    REQUIRE(pAggregated->GetInputs().size() == 3);
    REQUIRE(pAggregated->GetInputs() == inputs);

    std::vector<Output> outputs = tx1->GetOutputs();
    outputs.insert(outputs.end(), tx2->GetOutputs().begin(), tx2->GetOutputs().end());
    std::sort(outputs.begin(), outputs.end(), SortByHash);
    REQUIRE(pAggregated->GetOutputs().size() == 2);
    REQUIRE(pAggregated->GetOutputs() == outputs);

    std::vector<Kernel> kernels = tx1->GetKernels();
    kernels.insert(kernels.end(), tx2->GetKernels().begin(), tx2->GetKernels().end());
    std::sort(kernels.begin(), kernels.end(), SortByHash);
    REQUIRE(pAggregated->GetKernels().size() == 2);
    REQUIRE(pAggregated->GetKernels() == kernels);

    BlindingFactor offset = Crypto::AddBlindingFactors({ tx1->GetKernelOffset(), tx2->GetKernelOffset() });
    REQUIRE(pAggregated->GetKernelOffset() == offset);
}