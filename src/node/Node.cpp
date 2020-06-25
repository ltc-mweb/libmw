#include "Node.h"

#include <mw/node/validation/BlockValidator.h>
#include <mw/common/Logger.h>
#include <unordered_map>

MW_NAMESPACE

NODE_API mw::INode::Ptr InitializeNode(const FilePath& datadir, std::unordered_map<std::string, std::string>&& options)
{
    auto pConfig = NodeConfig::Create(datadir, std::move(options));
    auto database = BlockDBFactory::Open(pConfig->GetChainDir());
    auto pChainState = ChainState::Initialize(pConfig->GetDataDir(), pConfig, database);

    return std::shared_ptr<mw::INode>(new Node(pConfig, pChainState, database));
}

END_NAMESPACE

void Node::ValidateBlock(
    const Block::Ptr& pBlock,
    const std::vector<PegInCoin>& pegInCoins,
    const std::vector<PegOutCoin>& pegOutCoins) const
{
    LOG_TRACE_F("Validating block {}", pBlock);

    BlockValidator().Validate(pBlock, pegInCoins, pegOutCoins);
}

void Node::ConnectBlock(const Block::Ptr& pBlock, const mw::ICoinsView::Ptr& pView)
{
    LOG_TRACE_F("Connecting block {}", pBlock);

    auto pBatch = m_pChainState.BatchWrite();
    pBatch->ConnectBlock(pBlock);
    pBatch->Commit();
}

void Node::DisconnectBlock(const Block::CPtr& pBlock, const mw::ICoinsView::Ptr& pView)
{
    LOG_TRACE_F("Disconnecting block {}", pBlock);

    // TODO: Implement
}

ChainStatus::CPtr Node::GetStatus() const noexcept
{
    return m_pChainState.Read()->GetStatus();
}

Header::CPtr Node::GetHeader(const Hash& hash) const
{
    return m_pDatabase.Read()->GetHeaderByHash(hash);
}

Block::CPtr Node::GetBlock(const Hash& hash) const
{
    return m_pDatabase.Read()->GetBlockByHash(hash);
}