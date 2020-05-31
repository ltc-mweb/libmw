#include <test_framework/Node.h>

TEST_NAMESPACE

void Node::ValidateBlock(
    const Block::Ptr& pBlock,
    const std::vector<PegInCoin>& pegInCoins,
    const std::vector<PegOutCoin>& pegOutCoins) const
{

}

void Node::ConnectBlock(const Block::Ptr& pBlock)
{

}

void Node::DisconnectBlock(const Block::CPtr& pBlock)
{

}

ChainStatus::CPtr Node::GetStatus() const noexcept
{
    return nullptr;
}

Header::CPtr Node::GetHeader(const Hash& hash) const
{
    return nullptr;
}

Block::CPtr Node::GetBlock(const Hash& hash) const
{
    return nullptr;
}

END_NAMESPACE