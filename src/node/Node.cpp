#include "Node.h"
#include "CoinsViewFactory.h"

#include <mw/config/ChainParams.h>
#include <mw/node/validation/BlockValidator.h>
#include <mw/consensus/Aggregation.h>
#include <mw/common/Logger.h>
#include <mw/mmr/MMR.h>
#include <mw/mmr/backends/FileBackend.h>
#include <unordered_map>

MW_NAMESPACE

mw::INode::Ptr InitializeNode(
    const FilePath& datadir,
    const std::string& hrp,
    const mw::Header::CPtr& pBestHeader,
    const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper)
{
    auto pConfig = NodeConfig::Create(datadir, { });
    LoggerAPI::Initialize(pConfig->GetDataDir().GetChild("logs").CreateDirIfMissing(), "DEBUG"); // TODO: Read config

    mw::ChainParams::Initialize(hrp, 20); // TODO: Pass in pegin-maturity

    auto chain_dir = pConfig->GetChainDir();
    auto pLeafSet = mmr::LeafSet::Open(chain_dir);

    auto kernels_path = chain_dir.GetChild("kernels").CreateDirIfMissing();
    auto pKernelsBackend = mmr::FileBackend::Open('K', kernels_path, pDBWrapper);
    mmr::MMR::Ptr pKernelsMMR = std::make_shared<mmr::MMR>(pKernelsBackend);

    auto outputs_path = chain_dir.GetChild("outputs").CreateDirIfMissing();
    auto pOutputBackend = mmr::FileBackend::Open('O', outputs_path, pDBWrapper);
    mmr::MMR::Ptr pOutputMMR = std::make_shared<mmr::MMR>(pOutputBackend);

    auto rangeproof_path = chain_dir.GetChild("proofs").CreateDirIfMissing();
    auto pRangeProofBackend = mmr::FileBackend::Open('R', rangeproof_path, pDBWrapper);
    mmr::MMR::Ptr pRangeProofMMR = std::make_shared<mmr::MMR>(pRangeProofBackend);

    // TODO: Validate Current State
    mw::CoinsViewDB::Ptr pDBView = std::make_shared<mw::CoinsViewDB>(
        pBestHeader,
        pDBWrapper,
        pLeafSet,
        pKernelsMMR,
        pOutputMMR,
        pRangeProofMMR
    );

    return std::shared_ptr<mw::INode>(new Node(pConfig, pDBView));
}

END_NAMESPACE

Node::~Node()
{
    LoggerAPI::Shutdown();
}

void Node::ValidateBlock(
    const mw::Block::Ptr& pBlock,
    const std::vector<PegInCoin>& pegInCoins,
    const std::vector<PegOutCoin>& pegOutCoins) const
{
    assert(pBlock != nullptr);

    LOG_TRACE_F("Validating block {}", pBlock);
    BlockValidator().Validate(pBlock, pegInCoins, pegOutCoins);
    LOG_TRACE_F("Block {} validated", pBlock);
}

mw::BlockUndo::CPtr Node::ConnectBlock(const mw::Block::Ptr& pBlock, const mw::ICoinsView::Ptr& pView)
{
    assert(pBlock != nullptr);
    assert(pView != nullptr);

    LOG_TRACE_F("Connecting block {}", pBlock);

    mw::CoinsViewCache::Ptr pCache = std::make_shared<mw::CoinsViewCache>(pView);
    auto pUndo = pCache->ApplyBlock(pBlock);
    pCache->Flush(nullptr);

    LOG_TRACE_F("Block {} connected", pBlock);
    return pUndo;
}

void Node::DisconnectBlock(const mw::BlockUndo::CPtr& pUndoData, const mw::ICoinsView::Ptr& pView)
{
    assert(pUndoData != nullptr);
    assert(pView != nullptr);

    auto pHeader = pView->GetBestHeader();
    LOG_TRACE_F("Disconnecting block {}", pHeader);

    mw::CoinsViewCache::Ptr pCache = std::make_shared<mw::CoinsViewCache>(pView);
    pCache->UndoBlock(pUndoData);
    pCache->Flush(nullptr);

    LOG_TRACE_F("Block {} disconnected. New tip: {}", pHeader, pView->GetBestHeader());
}

mw::ICoinsView::Ptr Node::ApplyState(
    const libmw::IDBWrapper::Ptr& pDBWrapper,
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