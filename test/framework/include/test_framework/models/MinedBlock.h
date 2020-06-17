#pragma once

#include <mw/common/Macros.h>
#include <mw/models/block/Block.h>

#include <test_framework/models/Tx.h>

TEST_NAMESPACE

class MinedBlock
{
public:
    MinedBlock(const Block::Ptr& pBlock, std::vector<Tx>&& txs)
        : m_pBlock(pBlock), m_txs(std::move(txs)) { }
    MinedBlock(const Block::Ptr& pBlock, const std::vector<Tx>& txs)
        : m_pBlock(pBlock), m_txs(txs) { }

    const Header::CPtr& GetHeader() const noexcept { return m_pBlock->GetHeader(); }
    const Block::Ptr& GetBlock() const noexcept { return m_pBlock; }

private:
    Block::Ptr m_pBlock;
    std::vector<Tx> m_txs;
};

END_NAMESPACE