#include <mw/node/validation/BlockValidator.h>
#include <mw/exceptions/ValidationException.h>
#include <unordered_map>

void BlockValidator::Validate(
    const mw::Block::Ptr& pBlock,
    const mw::Hash& mweb_hash,
    const std::vector<PegInCoin>& pegInCoins,
    const std::vector<PegOutCoin>& pegOutCoins)
{
    assert(pBlock != nullptr);

    if (pBlock->WasValidated()) {
        return;
    }

    if (pBlock->GetHash() != mweb_hash) {
        std::cout << "Hash mismatch! Expected " << mweb_hash.ToHex() << " but was " << pBlock->GetHash().ToHex() << std::endl;
        // TODO: Enforce this in the next testnet
        // ThrowValidation(EConsensusError::HASH_MISMATCH);
    }

    pBlock->Validate();

    ValidatePegInCoins(pBlock, pegInCoins);
    ValidatePegOutCoins(pBlock, pegOutCoins);

    pBlock->MarkAsValidated();
}

void BlockValidator::ValidatePegInCoins(
    const mw::Block::CPtr& pBlock,
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

    for (const auto& kernel : pegInKernels) {
        auto pIter = pegInAmounts.find(kernel.GetCommitment());
        if (pIter == pegInAmounts.end() || kernel.GetAmount() != pIter->second) {
            ThrowValidation(EConsensusError::PEGIN_MISMATCH);
        }
    }
}

void BlockValidator::ValidatePegOutCoins(
    const mw::Block::CPtr& pBlock,
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

    for (const auto& kernel : pegOutKernels) {
        auto pIter = pegOutAmounts.find(kernel.GetAddress().value());
        if (kernel.GetAmount() != pIter->second) {
            ThrowValidation(EConsensusError::PEGOUT_MISMATCH);
        }
    }
}