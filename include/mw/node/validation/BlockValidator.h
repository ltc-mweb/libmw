#pragma once

#include <mw/models/block/Block.h>

class BlockValidator
{
public:
    BlockValidator() = default;

    void Validate(
        const Block::Ptr& pBlock,
        const std::vector<PegInCoin>& pegInCoins,
        const std::vector<PegOutCoin>& pegOutCoins
    );

private:
    void ValidatePegInCoins(
        const Block::CPtr& pBlock,
        const std::vector<PegInCoin>& pegInCoins
    );

    void ValidatePegOutCoins(
        const Block::CPtr& pBlock,
        const std::vector<PegOutCoin>& pegOutCoins
    );
};