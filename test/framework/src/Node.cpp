#include <test_framework/Node.h>
#include <test_framework/TestUtil.h>

TEST_NAMESPACE

Node::Ptr Node::Create()
{
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

mw::Header::CPtr Node::GetHeader(const mw::Hash& hash) const
{
    return m_pNode->GetHeader(hash);
}

mw::Block::CPtr Node::GetBlock(const mw::Hash& hash) const
{
    return m_pNode->GetBlock(hash);
}

END_NAMESPACE