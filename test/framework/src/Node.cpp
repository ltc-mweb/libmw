#include <test_framework/Node.h>
#include <test_framework/TestUtil.h>

TEST_NAMESPACE

Node::Ptr Node::Create()
{
    // TODO: DBWrapper
    auto pNode = mw::InitializeNode(test::TestUtil::GetTempDir(), { });
    return std::shared_ptr<Node>(new Node(pNode));
}

void Node::ValidateBlock(
    const mw::Block::Ptr& pBlock,
    const std::vector<PegInCoin>& pegInCoins,
    const std::vector<PegOutCoin>& pegOutCoins) const
{
    m_pNode->ValidateBlock(pBlock, pegInCoins, pegOutCoins);
}

void Node::ConnectBlock(const mw::Block::Ptr& pBlock, const mw::ICoinsView::Ptr& pView)
{
    m_pNode->ConnectBlock(pBlock, pView);
}

void Node::DisconnectBlock(const mw::Block::CPtr& pBlock, const mw::ICoinsView::Ptr& pView)
{
    m_pNode->DisconnectBlock(pBlock, pView);
}

ChainStatus::CPtr Node::GetStatus() const noexcept
{
    return m_pNode->GetStatus();
}

mw::ICoinsView::Ptr Node::ApplyState(
    const std::shared_ptr<mw::db::IDBWrapper>& pDBWrapper,
    const mw::IBlockStore& blockStore,
    const mw::Hash& firstMWHeaderHash,
    const mw::Hash& stateHeaderHash,
	const std::vector<UTXO::CPtr>& utxos,
	const std::vector<Kernel>& kernels)
{
    return m_pNode->ApplyState(pDBWrapper, blockStore, firstMWHeaderHash, stateHeaderHash, utxos, kernels);
}

END_NAMESPACE