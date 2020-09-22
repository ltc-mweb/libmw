#include "Node.h"
#include "CoinsViewFactory.h"

#include <mw/config/ChainParams.h>
#include <mw/node/validation/BlockValidator.h>
#include <mw/consensus/Aggregation.h>
#include <mw/common/Logger.h>
#include <mw/mmr/MMR.h>
#include <unordered_map>

MW_NAMESPACE

mw::INode::Ptr InitializeNode(const FilePath& datadir, const std::shared_ptr<mw::db::IDBWrapper>& pDBWrapper)
{
    auto pConfig = NodeConfig::Create(datadir, { });
    mw::ChainParams::Initialize("tmwltc"); // TODO: Pass this in

    // TODO: Validate Current State & Create DB View
    mw::CoinsViewDB::Ptr pDBView = nullptr;

    return std::shared_ptr<mw::INode>(new Node(pConfig, pDBView));
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

    mw::CoinsViewCache::Ptr pCache = std::make_shared<mw::CoinsViewCache>(pView);
    pCache->ApplyUpdates(pBlock->GetTxBody());
    // TODO: Check MMRs
    pView->SetBestHeader(pBlock->GetHeader());
    // TODO: If MMRs are correct, apply changes from pCache to pView
}

void Node::DisconnectBlock(const mw::Block::CPtr& pBlock, const mw::ICoinsView::Ptr& pView)
{
    LOG_TRACE_F("Disconnecting block {}", pBlock);

    // TODO: Implement
}

ChainStatus::CPtr Node::GetStatus() const noexcept
{
    // TODO: Implement
    return nullptr;
}

mw::ICoinsView::Ptr Node::ApplyState(
	const std::shared_ptr<mw::db::IDBWrapper>& pDBWrapper,
    const mw::IBlockStore& blockStore,
    const mw::Hash& firstMWHeaderHash,
    const mw::Hash& stateHeaderHash,
    const std::vector<UTXO::CPtr>& utxos,
    const std::vector<Kernel>& kernels)
{
    return CoinsViewFactory::CreateDBView(
        pDBWrapper,
        blockStore,
        m_pConfig->GetChainDir(),
        firstMWHeaderHash,
        stateHeaderHash,
        utxos,
        kernels
    );
}