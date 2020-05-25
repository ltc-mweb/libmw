#pragma once

#include <mw/ltc/models/block/Block.h>

class BlockValidator
{
public:
    BlockValidator(const Context::CPtr& pContext)
        : m_pContext(pContext) { }

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

    Context::CPtr m_pContext;
};