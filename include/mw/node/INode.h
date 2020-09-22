#pragma once

#include <mw/common/Macros.h>
#include <mw/common/ImportExport.h>
#include <mw/db/IBlockStore.h>
#include <mw/node/NodeConfig.h>
#include <mw/node/CoinsView.h>
#include <mw/models/block/Header.h>
#include <mw/models/block/Block.h>
#include <mw/models/tx/Transaction.h>
#include <mw/models/tx/PegInCoin.h>
#include <mw/models/tx/PegOutCoin.h>
#include <mw/models/tx/UTXO.h>
#include <mw/models/chain/ChainStatus.h>
#include <ext/interfaces.h>
#include <functional>
#include <vector>

// Forward Declarations
class CDBWrapper;

MW_NAMESPACE

class INode
{
public:
    using Ptr = std::shared_ptr<INode>;

    virtual ~INode() = default;

    virtual ICoinsView::Ptr GetDBView() = 0;

    //
    // Context-free validation of a block.
    //
    virtual void ValidateBlock(
        const mw::Block::Ptr& pBlock,
        const std::vector<PegInCoin>& pegInCoins,
        const std::vector<PegOutCoin>& pegOutCoins
    ) const = 0;

    //
    // Contextual validation of the block and application of the block to the supplied ICoinsView.
    // Consumer is required to call ValidateBlock first.
    //
    virtual void ConnectBlock(const mw::Block::Ptr& pBlock, const ICoinsView::Ptr& pView) = 0;

    virtual void DisconnectBlock(const mw::Block::CPtr& pBlock, const ICoinsView::Ptr& pView) = 0;

    virtual ChainStatus::CPtr GetStatus() const noexcept = 0;

    virtual mw::ICoinsView::Ptr ApplyState(
        const std::shared_ptr<mw::db::IDBWrapper>& pDBWrapper,
        const mw::IBlockStore& blockStore,
        const mw::Hash& firstMWHeaderHash,
        const mw::Hash& stateHeaderHash,
        const std::vector<UTXO::CPtr>& utxos,
        const std::vector<Kernel>& kernels
    ) = 0;
};

//
// Creates an instance of the node.
// This will fail if an instance is already running.
//
INode::Ptr InitializeNode(const FilePath& datadir, const std::shared_ptr<mw::db::IDBWrapper>& pDBWrapper);

END_NAMESPACE