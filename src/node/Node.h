#pragma once

#include <mw/node/INode.h>
#include <mw/common/Lock.h>

class Node : public mw::INode
{
public:
    Node(const NodeConfig::Ptr& pConfig, const mw::CoinsViewDB::Ptr& pDBView)
        : m_pConfig(pConfig), m_pDBView(pDBView) { }
    ~Node();

    mw::CoinsViewDB::Ptr GetDBView() final { return m_pDBView; }

    void ValidateBlock(
        const mw::Block::Ptr& pBlock,
        const mw::Hash& mweb_hash,
        const std::vector<PegInCoin>& pegInCoins,
        const std::vector<PegOutCoin>& pegOutCoins
    ) const final;

    mw::BlockUndo::CPtr ConnectBlock(const mw::Block::Ptr& pBlock, const mw::ICoinsView::Ptr& pView) final;
    void DisconnectBlock(const mw::BlockUndo::CPtr& pUndoData, const mw::ICoinsView::Ptr& pView) final;

    mw::ICoinsView::Ptr ApplyState(
        const libmw::IDBWrapper::Ptr& pDBWrapper,
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