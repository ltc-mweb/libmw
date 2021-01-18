#include <catch.hpp>

#include <mw/consensus/Weight.h>

#include <test_framework/models/Tx.h>
#include <test_framework/TxBuilder.h>

TEST_CASE("Weight::ExceedsMaximum")
{
    test::TxBuilder builder;
    for (size_t i = 0; i < 1000; i++) {
        builder.AddInput(1'001'000);
        builder.AddOutput(1'000'000);
        builder.AddPlainKernel(1'000, true);
    }

    // 1,000 outputs - 1,000 kernels - 1,000 owner_sigs = 21,000 Weight
    auto pTx1 = builder.Build();
    REQUIRE(Weight::Calculate(pTx1->GetBody()) == 21'000);
    REQUIRE_FALSE(Weight::ExceedsMaximum(pTx1->GetBody()));

    // 1,000 outputs - 1,001 kernels - 1,000 owner_sigs = 21,002 Weight
    auto pTx2 = builder.AddInput(1'000).AddPlainKernel(1'000).Build();
    REQUIRE(Weight::Calculate(pTx2->GetBody()) == 21'002);
    REQUIRE(Weight::ExceedsMaximum(pTx2->GetBody()));
}