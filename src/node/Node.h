#pragma once

#include <mw/node/INode.h>
#include <mw/common/Lock.h>

class Node : public mw::INode
{
public:
    Node(const NodeConfig::Ptr& pConfig, const mw::CoinsViewDB::Ptr& pDBView)
        : m_pConfig(pConfig), m_pDBView(pDBView) { }

    mw::CoinsViewDB::Ptr GetDBView() final { return m_pDBView; }

    void ValidateBlock(
        const mw::Block::Ptr& pBlock,
        const std::vector<PegInCoin>& pegInCoins,
        const std::vector<PegOutCoin>& pegOutCoins
    ) const final;

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
    NodeConfig ::Ptr m_pConfig;
    mw::CoinsViewDB::Ptr m_pDBView;
};