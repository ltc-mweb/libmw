#pragma once

#include <mw/common/Macros.h>
#include <mw/models/block/Header.h>
#include <mw/models/tx/Transaction.h>
#include <mw/consensus/Aggregation.h>
#include <mw/mmr/MMR.h>
#include <mw/mmr/backends/VectorBackend.h>

#include <test_framework/Node.h>
#include <test_framework/models/MinedBlock.h>
#include <test_framework/models/Tx.h>

#include <iostream>

TEST_NAMESPACE

class TestChain
{
public:
    TestChain(const Node::Ptr& pNode)
        : m_pNode(pNode) { }

    std::vector<MinedBlock> MineChain(const uint64_t totalHeight)
    {
        for (size_t i = 1; i <= totalHeight; i++)
        {
            MinedBlock block = AddNextBlock({ });

            m_pNode->ConnectBlock(block.GetBlock());
        }

        return m_blocks;
    }

    MinedBlock AddNextBlock(const std::vector<Tx>& txs, const uint64_t additionalDifficulty = 0)
    {
        Transaction::CPtr pTransaction = std::make_shared<const Transaction>();
        if (!txs.empty()) {
            std::vector<Transaction::CPtr> transactions;
            std::transform(
                txs.cbegin(), txs.cend(),
                std::back_inserter(transactions),
                [](const Tx& tx) { return tx.GetTransaction(); }
            );
            pTransaction = Aggregation::Aggregate(transactions);
        }

        auto pPrevHeader = m_blocks.back().GetHeader();
        auto kernelMMR = GetKernelMMR(pTransaction->GetKernels());
        auto outputMMR = GetOutputMMR(pTransaction->GetOutputs());
        auto rangeProofMMR = GetRangeProofMMR(pTransaction->GetOutputs());

        auto pHeader = std::make_shared<Header>(
            pPrevHeader->GetHeight() + 1,
            outputMMR.Root(),
            rangeProofMMR.Root(),
            kernelMMR.Root(),
            Crypto::AddBlindingFactors({ pPrevHeader->GetOffset(), pTransaction->GetOffset() }),
            outputMMR.GetNumNodes(),
            kernelMMR.GetNumNodes()
        );

        std::cout << "Mined Block: " << pHeader->GetHeight() << " - " << pHeader->Format() << std::endl;

        MinedBlock minedBlock(
            std::make_shared<Block>(pHeader, pTransaction->GetBody()),
            txs
        );
        m_blocks.push_back(minedBlock);

        return minedBlock;
    }

    void Rewind(const size_t nextHeight)
    {
        m_blocks.erase(m_blocks.begin() + nextHeight, m_blocks.end());
    }

private:
    mmr::MMR GetKernelMMR(const std::vector<Kernel>& additionalKernels = {})
    {
        std::vector<Kernel> kernels;
        for (const auto& block : m_blocks) {
            const auto& blockKernels = block.GetBlock()->GetKernels();
            std::copy(blockKernels.cbegin(), blockKernels.cend(), std::back_inserter(kernels));
        }

        kernels.insert(kernels.end(), additionalKernels.cbegin(), additionalKernels.cend());

        auto mmr = mmr::MMR(std::make_shared<mmr::VectorBackend>());
        for (const Kernel& kernel : kernels) {
            mmr.Add(kernel.Serialized());
        }

        return mmr;
    }

    mmr::MMR GetOutputMMR(const std::vector<Output>& additionalOutputs = {})
    {
        std::vector<Output> outputs;
        for (const auto& block : m_blocks) {
            const auto& blockOutputs = block.GetBlock()->GetOutputs();
            std::copy(blockOutputs.cbegin(), blockOutputs.cend(), std::back_inserter(outputs));
        }

        std::copy(additionalOutputs.cbegin(), additionalOutputs.cend(), std::back_inserter(outputs));

        auto mmr = mmr::MMR(std::make_shared<mmr::VectorBackend>());
        for (const Output& output : outputs) {
            mmr.Add(output.ToIdentifier().Serialized());
        }

        return mmr;
    }

    mmr::MMR GetRangeProofMMR(const std::vector<Output>& additionalOutputs = {})
    {
        std::vector<Output> outputs;
        for (const auto& block : m_blocks) {
            const auto& blockOutputs = block.GetBlock()->GetOutputs();
            std::copy(blockOutputs.cbegin(), blockOutputs.cend(), std::back_inserter(outputs));
        }

        std::copy(additionalOutputs.cbegin(), additionalOutputs.cend(), std::back_inserter(outputs));

        auto mmr = mmr::MMR(std::make_shared<mmr::VectorBackend>());
        for (const Output& output : outputs) {
            mmr.Add(output.GetRangeProof()->Serialized());
        }

        return mmr;
    }

    Node::Ptr m_pNode;
    std::vector<MinedBlock> m_blocks;
};

END_NAMESPACE