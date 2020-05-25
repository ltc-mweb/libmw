#include <mw/ltc/node/validation/BlockValidator.h>
#include <mw/core/exceptions/ValidationException.h>
#include <unordered_map>

void BlockValidator::Validate(
    const Block::Ptr& pBlock,
    const std::vector<PegInCoin>& pegInCoins,
    const std::vector<PegOutCoin>& pegOutCoins)
{
    if (pBlock->WasValidated()) {
        return;
    }

    pBlock->Validate(m_pContext);

    ValidatePegInCoins(pBlock, pegInCoins);
    ValidatePegOutCoins(pBlock, pegOutCoins);

    pBlock->MarkAsValidated();
}

void BlockValidator::ValidatePegInCoins(
    const Block::CPtr& pBlock,
    const std::vector<PegInCoin>& pegInCoins)
{
    std::unordered_map<Commitment, uint64_t> pegInAmounts;
    std::for_each(
        pegInCoins.cbegin(), pegInCoins.cend(),
        [&pegInAmounts](const PegInCoin& coin) {
            pegInAmounts.insert({ coin.GetCommitment(), coin.GetAmount() });
        }
    );

    auto pegInKernels = pBlock->GetPegInKernels();
    if (pegInKernels.size() != pegInAmounts.size()) {
        ThrowValidation(EConsensusError::PEGIN_MISMATCH);
    }

    for (auto pKernel : pegInKernels) {
        auto pIter = pegInAmounts.find(pKernel->GetCommitment());
        if (pKernel->GetAmount() != pIter->second) {
            ThrowValidation(EConsensusError::PEGIN_MISMATCH);
        }
    }
}

void BlockValidator::ValidatePegOutCoins(
    const Block::CPtr& pBlock,
    const std::vector<PegOutCoin>& pegOutCoins)
{
    std::unordered_map<Bech32Address, uint64_t> pegOutAmounts;
    std::for_each(
        pegOutCoins.cbegin(), pegOutCoins.cend(),
        [&pegOutAmounts](const PegOutCoin& coin) {
            pegOutAmounts.insert({ coin.GetAddress(), coin.GetAmount() });
        }
    );

    auto pegOutKernels = pBlock->GetPegOutKernels();
    if (pegOutKernels.size() != pegOutAmounts.size()) {
        ThrowValidation(EConsensusError::PEGOUT_MISMATCH);
    }

    for (auto pKernel : pegOutKernels) {
        auto pIter = pegOutAmounts.find(pKernel->GetAddress().value());
        if (pKernel->GetAmount() != pIter->second) {
            ThrowValidation(EConsensusError::PEGOUT_MISMATCH);
        }
    }
}