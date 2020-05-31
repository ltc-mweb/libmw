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
        const Block::Ptr& pBlock,
        const std::vector<PegInCoin>& pegInCoins,
        const std::vector<PegOutCoin>& pegOutCoins
    ) const final;

    void ConnectBlock(const Block::Ptr& pBlock) final;
    void DisconnectBlock(const Block::CPtr& pBlock) final;

    ChainStatus::CPtr GetStatus() const noexcept final;
    Header::CPtr GetHeader(const Hash& hash) const final;
    Block::CPtr GetBlock(const Hash& hash) const final;

private:
    NodeConfig ::Ptr m_pConfig;
    Locked<ChainState> m_pChainState;
    Locked<IBlockDB> m_pDatabase;
};