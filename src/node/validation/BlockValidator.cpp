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
        ThrowValidation(EConsensusError::HASH_MISMATCH);
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

    auto pegin_coins = pBlock->GetPegIns();
    if (pegin_coins.size() != pegInAmounts.size()) {
        ThrowValidation(EConsensusError::PEGIN_MISMATCH);
    }

    for (const auto& pegin : pegin_coins) {
        auto pIter = pegInAmounts.find(pegin.GetCommitment());
        if (pIter == pegInAmounts.end() || pegin.GetAmount() != pIter->second) {
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

    auto pegout_coins = pBlock->GetPegOuts();
    if (pegout_coins.size() != pegOutAmounts.size()) {
        ThrowValidation(EConsensusError::PEGOUT_MISMATCH);
    }

    for (const auto& pegout : pegout_coins) {
        auto pIter = pegOutAmounts.find(pegout.GetAddress());
        if (pIter == pegOutAmounts.end() || pegout.GetAmount() != pIter->second) {
            ThrowValidation(EConsensusError::PEGOUT_MISMATCH);
        }
    }
}