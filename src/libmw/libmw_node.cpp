#include <libmw/node.h>

#include "Transformers.h"
#include "BlockStoreWrapper.h"
#include "State.h"

#include <mw/consensus/BlockSumValidator.h>
#include <mw/exceptions/ValidationException.h>
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

MWEXPORT void Shutdown()
{
    LoggerAPI::Shutdown();
    NODE.reset();
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
    const libmw::BlockHash& hash,
    const std::vector<libmw::PegIn>& pegInCoins,
    const std::vector<libmw::PegOut>& pegOutCoins)
{
    auto mweb_hash = TransformHash(hash);
    auto pegins = TransformPegIns(pegInCoins);
    auto pegouts = TransformPegOuts(pegOutCoins);
    NODE->ValidateBlock(block.pBlock, mweb_hash, pegins, pegouts);
}

MWEXPORT libmw::BlockUndoRef ConnectBlock(const libmw::BlockRef& block, const CoinsViewRef& view)
{
    return libmw::BlockUndoRef{ NODE->ConnectBlock(block.pBlock, view.pCoinsView) };
}

MWEXPORT void DisconnectBlock(const libmw::BlockUndoRef& undoData, const CoinsViewRef& view)
{
    NODE->DisconnectBlock(undoData.pUndo, view.pCoinsView);
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

MWEXPORT void CheckTxInputs(const libmw::TxRef& transaction, const libmw::CoinsViewRef& view, int nSpendHeight)
{
    assert(transaction.pTransaction != nullptr);
    assert(view.pCoinsView != nullptr);

    auto pCoinsView = std::dynamic_pointer_cast<mw::CoinsViewCache>(view.pCoinsView);
    assert(pCoinsView != nullptr);

    for (const Input& input : transaction.pTransaction->GetInputs()) {
        auto utxos = pCoinsView->GetUTXOs(input.GetCommitment());
        if (utxos.empty()) {
            ThrowValidation(EConsensusError::UTXO_MISSING);
        }
        if (input.IsPeggedIn() && nSpendHeight - utxos.back()->GetBlockHeight() < mw::ChainParams::GetPegInMaturity()) {
            ThrowValidation(EConsensusError::PEGIN_MATURITY);
        }
    }
}

MWEXPORT bool HasCoin(const libmw::CoinsViewRef& view, const libmw::Commitment& commitment)
{
    assert(view.pCoinsView != nullptr);

    return !view.pCoinsView->GetUTXOs(BigInt<33>(commitment)).empty();
}

MWEXPORT bool HasCoinInCache(const libmw::CoinsViewRef& view, const libmw::Commitment& commitment)
{
    assert(view.pCoinsView != nullptr);

    auto pCoinsView = std::dynamic_pointer_cast<mw::CoinsViewCache>(view.pCoinsView);
    assert(pCoinsView != nullptr);

    return pCoinsView->HasCoinInCache(BigInt<33>(commitment));
}

END_NAMESPACE // node
END_NAMESPACE // libmw