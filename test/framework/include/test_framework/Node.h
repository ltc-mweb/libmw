#pragma once

#include <mw/common/Macros.h>
#include <mw/node/INode.h>

TEST_NAMESPACE

class Node : public mw::INode
{
public:
    using Ptr = std::shared_ptr<Node>;

    static Node::Ptr Create();

    void ValidateBlock(
        const Block::Ptr& pBlock,
        const std::vector<PegInCoin>& pegInCoins,
        const std::vector<PegOutCoin>& pegOutCoins
    ) const final;

    //
    // Contextual validation of the block and application of the block to the active chain.
    // Consumer is required to call ValidateBlock first.
    //
    void ConnectBlock(const Block::Ptr& pBlock, const mw::ICoinsView::Ptr& pView) final;

    void DisconnectBlock(const Block::CPtr& pBlock, const mw::ICoinsView::Ptr& pView) final;

    ChainStatus::CPtr GetStatus() const noexcept final;
    Header::CPtr GetHeader(const Hash& hash) const final;
    Block::CPtr GetBlock(const Hash& hash) const final;

private:
    Node(const mw::INode::Ptr& pNode)
        : m_pNode(pNode) { }

    mw::INode::Ptr m_pNode;
};

END_NAMESPACE