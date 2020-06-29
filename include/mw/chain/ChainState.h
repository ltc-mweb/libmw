#pragma once

#include <mw/traits/Batchable.h>
#include <mw/models/block/Block.h>
#include <mw/models/chain/ChainStatus.h>
#include <mw/chain/UTXOSet.h>
#include <mw/mmr/MMR.h>
#include <mw/mmr/backends/FileBackend.h>
#include <mw/db/IBlockDB.h>
#include <boost/optional.hpp>

class ChainState : public Traits::IBatchable
{
public:
    using Ptr = std::shared_ptr<ChainState>;

    ChainState(
        const NodeConfig::Ptr& pConfig,
        const ChainStatus::Ptr& pStatus,
        const mmr::MMR::Ptr& pKernelMMR,
        const UTXOSet::Ptr& pUTXOSet
    ) : m_pConfig(pConfig),
        m_pStatus(pStatus),
        m_pKernelMMR(pKernelMMR),
        m_pUTXOSet(pUTXOSet) { }

    static ChainState::Ptr Initialize(
        const FilePath& dataPath,
        const NodeConfig::Ptr& pConfig,
        const Locked<IBlockDB>& database)
    {
        auto pKernelBackend = mmr::FileBackend::Open(
            dataPath.GetChild("chain").GetChild("kernel"),
            boost::none
        );

        return std::make_shared<ChainState>(
            pConfig,
            std::make_shared<ChainStatus>(),
            std::make_shared<mmr::MMR>(pKernelBackend),
            UTXOSet::Initialize(pConfig)
        );
    }

    void ConnectBlock(const Block::CPtr& pBlock)
    {
        // TODO: Validate that this is the next block.
        // TODO: Handle reorgs?

        pBlock->Validate();

        const auto& kernels = pBlock->GetKernels();
        std::for_each(
            kernels.cbegin(), kernels.cend(),
            [this](const Kernel& kernel) { m_pKernelMMR->Add(kernel.Serialized()); }
        );

        m_pUTXOSet->ApplyBlock(pBlock);

        // TODO: Update mempool
    }

    ChainStatus::CPtr GetStatus() const noexcept { return m_pStatus; }

    void Commit() final
    {
        m_pKernelMMR->Commit();
        m_pUTXOSet->Commit();
    }

    void Rollback() noexcept final
    {
        m_pKernelMMR->Rollback();
        m_pUTXOSet->Rollback();
    }

private:
    NodeConfig::Ptr m_pConfig;
    ChainStatus::Ptr m_pStatus;
    mmr::MMR::Ptr m_pKernelMMR;
    UTXOSet::Ptr m_pUTXOSet;
};