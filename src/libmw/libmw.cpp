#include <ext/libmw.h>

#include "Transformers.h"
#include "BlockStoreWrapper.h"
#include "State.h"

#include <mw/models/block/Block.h>
#include <mw/models/tx/Transaction.h>
#include <mw/models/tx/UTXO.h>
#include <mw/node/INode.h>

static mw::INode::Ptr NODE = nullptr;

MW_NAMESPACE

EXPORT mw::HeaderRef DeserializeHeader(std::vector<uint8_t>&& bytes)
{
    Deserializer deserializer{ std::move(bytes) };
    auto pHeader = std::make_shared<mw::Header>(mw::Header::Deserialize(deserializer));
    return mw::HeaderRef{ pHeader };
}

EXPORT std::vector<uint8_t> SerializeHeader(const mw::HeaderRef& header)
{
    return header.pHeader->Serialized();
}

EXPORT mw::BlockRef DeserializeBlock(std::vector<uint8_t>&& bytes)
{
    Deserializer deserializer{ std::move(bytes) };
    auto pBlock = std::make_shared<mw::Block>(mw::Block::Deserialize(deserializer));
    return mw::BlockRef{ pBlock };
}

EXPORT std::vector<uint8_t> SerializeBlock(const mw::BlockRef& block)
{
    return block.pBlock->Serialized();
}

EXPORT uint64_t TxRef::GetTotalFee() const noexcept
{
    return pTransaction->GetTotalFee();
}

EXPORT mw::TxRef DeserializeTx(std::vector<uint8_t>&& bytes)
{
    Deserializer deserializer{ bytes };
    auto pTx = std::make_shared<mw::Transaction>(mw::Transaction::Deserialize(deserializer));
    return mw::TxRef{ pTx };
}

EXPORT std::vector<uint8_t> SerializeTx(const mw::TxRef& tx)
{
    return tx.pTransaction->Serialized();
}

EXPORT CoinsViewRef CoinsViewRef::CreateCache() const
{
    if (pCoinsView == nullptr) {
        return CoinsViewRef{ nullptr };
    }

    return CoinsViewRef{ std::make_shared<mw::CoinsViewCache>(pCoinsView) };
}

NODE_NAMESPACE

EXPORT mw::CoinsViewRef Initialize(const std::string& datadir, const std::shared_ptr<mw::db::IDBWrapper>& pDBWrapper)
{
    NODE = mw::InitializeNode(FilePath{ datadir }, pDBWrapper);

    return mw::CoinsViewRef{ NODE->GetDBView() };
}

EXPORT mw::CoinsViewRef ApplyState(
    const std::string& datadir,
    const mw::db::IBlockStore* pBlockStore,
    const mw::BlockHash& firstMWHeaderHash,
    const mw::BlockHash& stateHeaderHash,
    const std::shared_ptr<mw::db::IDBWrapper>& pDBWrapper,
    const mw::State& state)
{
    BlockStoreWrapper blockStore(pBlockStore);
    auto pCoinsViewDB = NODE->ApplyState(
        pDBWrapper,
        blockStore,
        mw::Hash{ firstMWHeaderHash },
        mw::Hash{ stateHeaderHash },
        state.utxos,
        state.kernels
    );

    return mw::CoinsViewRef{ pCoinsViewDB };
}

EXPORT void CheckBlock(const mw::BlockRef& block, const std::vector<mw::PegIn>& pegInCoins, const std::vector<mw::PegOut>& pegOutCoins)
{
    auto pegins = TransformPegIns(pegInCoins);
    auto pegouts = TransformPegOuts(pegOutCoins);
    NODE->ValidateBlock(block.pBlock, pegins, pegouts);
}

EXPORT void ConnectBlock(const mw::BlockRef& block, const CoinsViewRef& view)
{
    NODE->ConnectBlock(block.pBlock, view.pCoinsView);
}

EXPORT void DisconnectBlock(const mw::BlockRef& block, const CoinsViewRef& view)
{
    NODE->DisconnectBlock(block.pBlock, view.pCoinsView);
}

EXPORT mw::StateRef DeserializeState(std::vector<uint8_t>&& bytes)
{
    Deserializer deserializer{ std::move(bytes) };
    mw::State state = mw::State::Deserialize(deserializer);
    return { std::make_shared<mw::State>(std::move(state)) };
}

EXPORT std::vector<uint8_t> SerializeState(const mw::StateRef& state)
{
    return state.pState->Serialized();
}

EXPORT mw::StateRef SnapshotState(const mw::CoinsViewRef& view, const mw::BlockHash& block_hash)
{
    return { nullptr }; // TODO: Implement
}

EXPORT void CheckTransaction(const mw::TxRef& transaction)
{
    // TODO: Implement
}

END_NAMESPACE
END_NAMESPACE