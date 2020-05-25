#pragma once

#include <mw/ltc/node/INode.h>
#include <mw/ltc/chain/ChainState.h>
#include <mw/core/common/Lock.h>

class Node : public mw::ltc::INode
{
public:
    Node(const NodeConfig::Ptr& pConfig, const Context::CPtr& pContext, const ChainState::Ptr& pChainState, const Locked<IBlockDB>& database)
        : m_pConfig(pConfig), m_pContext(pContext), m_pChainState(pChainState), m_pDatabase(database) { }

    void ValidateBlock(
        const Block::Ptr& pBlock,
        const std::vector<PegInCoin>& pegInCoins,
        const std::vector<PegOutCoin>& pegOutCoins
    ) const final;

    void ConnectBlock(const Block::Ptr& pBlock) final;

    ChainStatus::CPtr GetStatus() const noexcept final;
    Header::CPtr GetHeader(const Hash& hash) const final;
    Block::CPtr GetBlock(const Hash& hash) const final;

private:
    NodeConfig ::Ptr m_pConfig;
    Context::CPtr m_pContext;
    Locked<ChainState> m_pChainState;
    Locked<IBlockDB> m_pDatabase;
};