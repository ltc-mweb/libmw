#pragma once

#include <mw/common/Macros.h>
#include <mw/node/INode.h>

TEST_NAMESPACE

class Node : public mw::INode
{
public:
    using Ptr = std::shared_ptr<Node>;

    static Node::Ptr Create();

    mw::ICoinsView::Ptr GetDBView() final { return m_pNode->GetDBView(); }

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

    mw::ICoinsView::Ptr ApplyState(
        const std::shared_ptr<mw::db::IDBWrapper>& pDBWrapper,
        const mw::IBlockStore& blockStore,
        const mw::Hash& firstMWHeaderHash,
        const mw::Hash& stateHeaderHash,
		const std::vector<UTXO::CPtr>& utxos,
		const std::vector<Kernel>& kernels
    ) final;

private:
    Node(const mw::INode::Ptr& pNode)
        : m_pNode(pNode) { }

    mw::INode::Ptr m_pNode;
};

END_NAMESPACE