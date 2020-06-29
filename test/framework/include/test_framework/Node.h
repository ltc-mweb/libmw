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
        const mw::Block::Ptr& pBlock,
        const std::vector<PegInCoin>& pegInCoins,
        const std::vector<PegOutCoin>& pegOutCoins
    ) const final;

    //
    // Contextual validation of the block and application of the block to the active chain.
    // Consumer is required to call ValidateBlock first.
    //
    void ConnectBlock(const mw::Block::Ptr& pBlock, const mw::ICoinsView::Ptr& pView) final;

    void DisconnectBlock(const mw::Block::CPtr& pBlock, const mw::ICoinsView::Ptr& pView) final;

    ChainStatus::CPtr GetStatus() const noexcept final;
    Header::CPtr GetHeader(const mw::Hash& hash) const final;
    mw::Block::CPtr GetBlock(const mw::Hash& hash) const final;

private:
    Node(const mw::INode::Ptr& pNode)
        : m_pNode(pNode) { }

    mw::INode::Ptr m_pNode;
};

END_NAMESPACE