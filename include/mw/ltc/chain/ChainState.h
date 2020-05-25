#pragma once

#include <mw/core/traits/Batchable.h>
#include <mw/ltc/models/block/Block.h>
#include <mw/ltc/models/chain/ChainStatus.h>
#include <mw/ltc/chain/UTXOSet.h>
#include <mw/core/mmr/MMR.h>
#include <mw/core/mmr/backends/FileBackend.h>
#include <mw/core/db/IBlockDB.h>

class ChainState : public Traits::IBatchable
{
public:
    using Ptr = std::shared_ptr<ChainState>;

    ChainState(
        const Context::CPtr& pContext,
        const ChainStatus::Ptr& pStatus,
        const mmr::MMR::Ptr& pKernelMMR,
        const UTXOSet::Ptr& pUTXOSet
    ) : m_pContext(pContext),
        m_pStatus(pStatus),
        m_pKernelMMR(pKernelMMR),
        m_pUTXOSet(pUTXOSet) { }

    static ChainState::Ptr Initialize(
        const FilePath& dataPath,
        const Context::CPtr& pContext,
        const Locked<IBlockDB>& database)
    {
        auto pKernelBackend = mmr::FileBackend::Open(
            dataPath.GetChild("chain").GetChild("kernel"),
            tl::nullopt
        );

        return std::make_shared<ChainState>(
            pContext,
            std::make_shared<ChainStatus>(),
            std::make_shared<mmr::MMR>(pKernelBackend),
            UTXOSet::Initialize(pContext)
        );
    }

    void ConnectBlock(const Block::CPtr& pBlock)
    {
        // TODO: Validate that this is the next block.
        // TODO: Handle reorgs?

        pBlock->Validate(m_pContext);

        const auto& kernels = pBlock->GetKernels();
        std::for_each(
            kernels.cbegin(), kernels.cend(),
            [this](const IKernel::CPtr& pKernel) { m_pKernelMMR->Add(pKernel->Serialized()); }
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
    Context::CPtr m_pContext;
    ChainStatus::Ptr m_pStatus;
    mmr::MMR::Ptr m_pKernelMMR;
    UTXOSet::Ptr m_pUTXOSet;
};