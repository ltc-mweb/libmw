#include <catch.hpp>

#include <mw/consensus/CutThrough.h>
#include <mw/crypto/Random.h>

#include <test_framework/models/Tx.h>

TEST_CASE("Cut-Through")
{
    BlindingFactor blind_a = Random::CSPRNG<32>();
    BlindingFactor blind_b = Random::CSPRNG<32>();
    BlindingFactor blind_c = Random::CSPRNG<32>();

    Input input1(EOutputFeatures::DEFAULT_OUTPUT, Crypto::CommitBlinded(50, blind_a));
    Input input2(EOutputFeatures::DEFAULT_OUTPUT, Crypto::CommitBlinded(25, blind_b));
    Input input3(EOutputFeatures::PEGGED_IN, Crypto::CommitBlinded(10, blind_c));

    Output output1 = test::TxOutput::Create(EOutputFeatures::DEFAULT_OUTPUT, blind_a, 50).GetOutput();
    Output output2 = test::TxOutput::Create(EOutputFeatures::DEFAULT_OUTPUT, blind_c, 25).GetOutput();
    Output output3 = test::TxOutput::Create(EOutputFeatures::PEGGED_IN, blind_c, 10).GetOutput();

    std::vector<Input> inputs{ input1, input2, input3 };
    std::vector<Output> outputs{ output1, output2, output3 };

    CutThrough::PerformCutThrough(inputs, outputs);

    REQUIRE(inputs == std::vector<Input>({ input2 }));
    REQUIRE(outputs == std::vector<Output>({ output2 }));

    CutThrough::VerifyCutThrough(inputs, outputs);
}