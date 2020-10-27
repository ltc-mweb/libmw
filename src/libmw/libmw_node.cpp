#include <libmw/node.h>

#include "Transformers.h"
#include "BlockStoreWrapper.h"
#include "State.h"

#include <mw/consensus/BlockSumValidator.h>
#include <mw/models/block/Block.h>
#include <mw/models/block/BlockUndo.h>
#include <mw/models/tx/Transaction.h>
#include <mw/models/tx/UTXO.h>
#include <mw/node/INode.h>
#include <mw/wallet/Wallet.h>

static mw::INode::Ptr NODE = nullptr;

LIBMW_NAMESPACE
NODE_NAMESPACE

MWEXPORT libmw::CoinsViewRef Initialize(
    const libmw::ChainParams& chainParams,
    const libmw::HeaderRef& header,
    const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper)
{
    NODE = mw::InitializeNode(FilePath{ chainParams.dataDirectory }, chainParams.hrp, header.pHeader, pDBWrapper);

    return libmw::CoinsViewRef{ NODE->GetDBView() };
}

MWEXPORT libmw::CoinsViewRef ApplyState(
    const libmw::IBlockStore::Ptr& pBlockStore,
    const libmw::IDBWrapper::Ptr& pCoinsDB,
    const libmw::BlockHash& firstMWHeaderHash,
    const libmw::BlockHash& stateHeaderHash,
    const libmw::StateRef& state)
{
    BlockStoreWrapper blockStore(pBlockStore.get());
    auto pCoinsViewDB = NODE->ApplyState(
        pCoinsDB,
        blockStore,
        mw::Hash{ firstMWHeaderHash },
        mw::Hash{ stateHeaderHash },
        state.pState->utxos,
        state.pState->kernels
    );

    return libmw::CoinsViewRef{ pCoinsViewDB };
}

MWEXPORT void CheckBlock(
    const libmw::BlockRef& block,
    const std::vector<libmw::PegIn>& pegInCoins,
    const std::vector<libmw::PegOut>& pegOutCoins)
{
    auto pegins = TransformPegIns(pegInCoins);
    auto pegouts = TransformPegOuts(pegOutCoins);
    NODE->ValidateBlock(block.pBlock, pegins, pegouts);
}

MWEXPORT libmw::BlockUndoRef ConnectBlock(const libmw::BlockRef& block, const CoinsViewRef& view)
{
    return libmw::BlockUndoRef{ NODE->ConnectBlock(block.pBlock, view.pCoinsView) };
}

MWEXPORT void DisconnectBlock(const libmw::BlockUndoRef& undoData, const CoinsViewRef& view)
{
    NODE->DisconnectBlock(undoData.pUndo, view.pCoinsView);
}

MWEXPORT libmw::BlockAndPegs BuildNextBlock(
    const uint64_t height,
    const libmw::CoinsViewRef& view,
    const std::vector<libmw::TxRef>& transactions)
{
    mw::CoinsViewCache viewCache(view.pCoinsView);

    LOG_TRACE_F("Building block with {} txs", transactions.size());
    auto txs = TransformTxs(transactions);
    auto pBlock = viewCache.BuildNextBlock(height, txs);
    LOG_TRACE_F("Next block built: {}", Json(pBlock->ToJSON()));

    return TransformBlock(pBlock);
}

MWEXPORT void FlushCache(const libmw::CoinsViewRef& view, const std::unique_ptr<libmw::IDBBatch>& pBatch)
{
    LOG_TRACE("Flushing cache");
    auto pViewCache = dynamic_cast<mw::CoinsViewCache*>(view.pCoinsView.get());
    assert(pViewCache != nullptr);

    pViewCache->Flush(pBatch);
    LOG_TRACE("Cache flushed");
}

MWEXPORT libmw::StateRef SnapshotState(const libmw::CoinsViewRef&)
{
    return { nullptr }; // TODO: Implement
}

MWEXPORT void CheckTransaction(const libmw::TxRef& transaction)
{
    assert(transaction.pTransaction != nullptr);

    transaction.pTransaction->GetBody().Validate();
    BlockSumValidator::ValidateForTx(*transaction.pTransaction);
}

END_NAMESPACE // node
END_NAMESPACE // libmw