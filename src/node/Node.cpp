#include "Node.h"

#include <mw/ltc/node/validation/BlockValidator.h>
#include <mw/core/common/Logger.h>
#include <unordered_map>

mw::ltc::INode::Ptr InitializeNode(const FilePath& datadir, std::unordered_map<std::string, std::string>&& options)
{
    auto pContext = Context::Create();
    auto pConfig = NodeConfig::Create(datadir, std::move(options));
    auto database = BlockDBFactory::Open(pContext, pConfig->GetChainDir());
    auto pChainState = ChainState::Initialize(pConfig->GetDataDir(), pContext, database);

    return std::shared_ptr<mw::ltc::INode>(new Node(pConfig, pContext, pChainState, database));
}

void Node::ValidateBlock(
    const Block::CPtr& pBlock,
    const std::vector<PegInCoin>& pegInCoins,
    const std::vector<PegOutCoin>& pegOutCoins) const
{
    LOG_TRACE_F("Validating block {}", pBlock);

    BlockValidator(m_pContext).Validate(pBlock, pegInCoins, pegOutCoins);
}

void Node::ConnectBlock(const Block::CPtr& pBlock)
{
    auto pBatch = m_pChainState.BatchWrite();
    pBatch->ConnectBlock(pBlock);
    pBatch->Commit();
}

ChainStatus::CPtr Node::GetStatus() const noexcept
{
    return m_pChainState.Read()->GetStatus();
}

Header::CPtr Node::GetHeader(const Hash& hash) const
{
    return std::dynamic_pointer_cast<const Header, const IHeader>(
        m_pDatabase.Read()->GetHeaderByHash(hash)
    );
}

Block::CPtr Node::GetBlock(const Hash& hash) const
{
    return std::dynamic_pointer_cast<const Block, const IBlock>(
        m_pDatabase.Read()->GetBlockByHash(hash)
    );
}