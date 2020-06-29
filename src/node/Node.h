#pragma once

#include <mw/node/INode.h>
#include <mw/chain/ChainState.h>
#include <mw/common/Lock.h>

class Node : public mw::INode
{
public:
    Node(const NodeConfig::Ptr& pConfig, const ChainState::Ptr& pChainState, const Locked<IBlockDB>& database)
        : m_pConfig(pConfig), m_pChainState(pChainState), m_pDatabase(database) { }

    void ValidateBlock(
        const mw::Block::Ptr& pBlock,
        const std::vector<PegInCoin>& pegInCoins,
        const std::vector<PegOutCoin>& pegOutCoins
    ) const final;

    void ConnectBlock(const mw::Block::Ptr& pBlock, const mw::ICoinsView::Ptr& pView) final;
    void DisconnectBlock(const mw::Block::CPtr& pBlock, const mw::ICoinsView::Ptr& pView) final;

    ChainStatus::CPtr GetStatus() const noexcept final;
    mw::Header::CPtr GetHeader(const mw::Hash& hash) const final;
    mw::Block::CPtr GetBlock(const mw::Hash& hash) const final;

private:
    NodeConfig ::Ptr m_pConfig;
    Locked<ChainState> m_pChainState;
    Locked<IBlockDB> m_pDatabase;
};