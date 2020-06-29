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
    const mw::Block::Ptr& pBlock,
    const std::vector<PegInCoin>& pegInCoins,
    const std::vector<PegOutCoin>& pegOutCoins) const
{
    LOG_TRACE_F("Validating block {}", pBlock);

    BlockValidator().Validate(pBlock, pegInCoins, pegOutCoins);
}

void Node::ConnectBlock(const mw::Block::Ptr& pBlock, const mw::ICoinsView::Ptr& pView)
{
    LOG_TRACE_F("Connecting block {}", pBlock);

    auto pBatch = m_pChainState.BatchWrite();
    pBatch->ConnectBlock(pBlock);
    pBatch->Commit();
}

void Node::DisconnectBlock(const mw::Block::CPtr& pBlock, const mw::ICoinsView::Ptr& pView)
{
    LOG_TRACE_F("Disconnecting block {}", pBlock);

    // TODO: Implement
}

ChainStatus::CPtr Node::GetStatus() const noexcept
{
    return m_pChainState.Read()->GetStatus();
}

mw::Header::CPtr Node::GetHeader(const mw::Hash& hash) const
{
    return m_pDatabase.Read()->GetHeaderByHash(hash);
}

mw::Block::CPtr Node::GetBlock(const mw::Hash& hash) const
{
    return m_pDatabase.Read()->GetBlockByHash(hash);
}