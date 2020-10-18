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

    mw::ChainParams::Initialize(hrp);

    auto chainDir = pConfig->GetChainDir();
    auto pLeafSet = mmr::LeafSet::Open(chainDir);

    auto pKernelsBackend = mmr::FileBackend::Open("kernels", chainDir, pDBWrapper);
    mmr::MMR::Ptr pKernelsMMR = std::make_shared<mmr::MMR>(pKernelsBackend);

    auto pOutputBackend = mmr::FileBackend::Open("outputs", chainDir, pDBWrapper);
    mmr::MMR::Ptr pOutputMMR = std::make_shared<mmr::MMR>(pOutputBackend);

    auto pRangeProofBackend = mmr::FileBackend::Open("proofs", chainDir, pDBWrapper);
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
    LOG_TRACE_F("Validating block {}", pBlock);
    BlockValidator().Validate(pBlock, pegInCoins, pegOutCoins);
    LOG_TRACE_F("Block {} validated", pBlock);
}

mw::BlockUndo::CPtr Node::ConnectBlock(const mw::Block::Ptr& pBlock, const mw::ICoinsView::Ptr& pView)
{
    LOG_TRACE_F("Connecting block {}", pBlock);

    mw::CoinsViewCache::Ptr pCache = std::make_shared<mw::CoinsViewCache>(pView);
    auto pUndo = pCache->ApplyBlock(pBlock);
    pCache->Flush(nullptr);

    LOG_TRACE_F("Block {} connected", pBlock);
    return pUndo;
}

void Node::DisconnectBlock(const mw::BlockUndo::CPtr& pUndoData, const mw::ICoinsView::Ptr& pView)
{
    auto pHeader = pView->GetBestHeader();
    //assert(pHeader != nullptr);

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