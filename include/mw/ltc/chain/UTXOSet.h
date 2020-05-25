#pragma once

#include <mw/core/traits/Batchable.h>
#include <mw/ltc/models/block/Block.h>
#include <mw/core/mmr/MMR.h>
#include <mw/core/mmr/LeafSet.h>

class UTXOSet : public Traits::IBatchable
{
public:
    using Ptr = std::shared_ptr<UTXOSet>;

    static UTXOSet::Ptr Initialize(const Context::CPtr& pContext)
    {
        // TODO: Implement
        return nullptr;
    }

    void ApplyBlock(const Block::CPtr& pBlock)
    {
        const auto& inputs = pBlock->GetInputs();

        std::unordered_map<Commitment, mmr::LeafIndex> leafIndices; // TODO: Lookup

        std::for_each(
            inputs.cbegin(), inputs.cend(),
            [this, leafIndices](const Input& input) {
                auto iter = leafIndices.find(input.GetCommitment());
                if (iter == leafIndices.cend())
                {
                    // TODO: Throw exception
                }

                m_pLeafSet->Remove(iter->second);
            }
        );

        const auto& outputs = pBlock->GetOutputs();
        std::for_each(
            outputs.cbegin(), outputs.cend(),
            [this](const Output& output) {
                const mmr::LeafIndex index = mmr::LeafIndex::At(m_pLeafSet->GetSize());
                m_pLeafSet->Add(index);
                m_pOutputPMMR->Add(output.Serialized());
            }
        );
    }

    void Validate(const Block::CPtr& pBlock)
    {
        const auto& inputs = pBlock->GetInputs();

        std::unordered_map<Commitment, mmr::LeafIndex> leafIndices; // TODO: Lookup

        std::for_each(
            inputs.cbegin(), inputs.cend(),
            [this, leafIndices](const Input& input) {
                auto iter = leafIndices.find(input.GetCommitment());
                if (iter == leafIndices.cend() || !m_pLeafSet->Contains(iter->second))
                {
                    // TODO: Throw exception
                }
            }
        );
    }

    void Commit() final
    {
        m_pLeafSet->Commit();
        m_pOutputPMMR->Commit();
        m_pRangeProofPMMR->Commit();
    }

    void Rollback() noexcept final
    {
        m_pLeafSet->Rollback();
        m_pOutputPMMR->Rollback();
        m_pRangeProofPMMR->Rollback();
    }

private:
    UTXOSet(
        const Context::CPtr& pContext,
        const mmr::LeafSet::Ptr& pLeafSet,
        const mmr::MMR::Ptr& pOutputPMMR,
        const mmr::MMR::Ptr& pRangeProofPMMR
    ) : m_pContext(pContext),
        m_pLeafSet(pLeafSet),
        m_pOutputPMMR(pOutputPMMR),
        m_pRangeProofPMMR(pRangeProofPMMR) { }

    void SpendInput(const Input& input)
    {

    }

    Context::CPtr m_pContext;
    mmr::LeafSet::Ptr m_pLeafSet;
    mmr::MMR::Ptr m_pOutputPMMR;
    mmr::MMR::Ptr m_pRangeProofPMMR;
};