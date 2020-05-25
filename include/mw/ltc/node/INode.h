#pragma once

#include <mw/ltc/node/NodeConfig.h>
#include <mw/ltc/models/block/Header.h>
#include <mw/ltc/models/block/Block.h>
#include <mw/ltc/models/tx/PegInCoin.h>
#include <mw/ltc/models/tx/PegOutCoin.h>
#include <mw/ltc/models/chain/ChainStatus.h>
#include <mw/core/db/IBlockDB.h>
#include <vector>

namespace mw
{
namespace ltc
{

class INode
{
public:
    using Ptr = std::shared_ptr<INode>;

    virtual ~INode() = default;

    //
    // Context-free validation of a block.
    // Adds the block to the orphan pool.
    //
    virtual void ValidateBlock(
        const Block::Ptr& pBlock,
        const std::vector<PegInCoin>& pegInCoins,
        const std::vector<PegOutCoin>& pegOutCoins
    ) const = 0;

    //
    // Contextual validation of the block and application of the block to the active chain.
    // Consumer is required to call ValidateBlock first.
    //
    virtual void ConnectBlock(const Block::Ptr& pBlock) = 0;

    virtual ChainStatus::CPtr GetStatus() const noexcept = 0;
    virtual Header::CPtr GetHeader(const Hash& hash) const = 0;
    virtual Block::CPtr GetBlock(const Hash& hash) const = 0;
};

//
// Creates an instance of the node.
// This will fail if an instance is already running.
//
INode::Ptr InitializeNode(const FilePath& datadir, std::unordered_map<std::string, std::string>&& options);

}
}