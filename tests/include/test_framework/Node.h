#pragma once

#include <mw/ltc/node/INode.h>

namespace test
{
class Node : public mw::ltc::INode
{
    void ValidateBlock(
        const Block::CPtr& pBlock,
        const std::vector<PegInCoin>& pegInCoins,
        const std::vector<PegOutCoin>& pegOutCoins
    ) const final;

    //
    // Contextual validation of the block and application of the block to the active chain.
    // Consumer is required to call ValidateBlock first.
    //
    void ConnectBlock(const Block::CPtr& pBlock) final;

    ChainStatus::CPtr GetStatus() const noexcept final;
    Header::CPtr GetHeader(const uint64_t height) const final;
    Block::CPtr GetBlock(const uint64_t height) const final;
};
}