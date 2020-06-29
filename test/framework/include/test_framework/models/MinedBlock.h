#pragma once

#include <mw/common/Macros.h>
#include <mw/models/block/Block.h>

#include <test_framework/models/Tx.h>

TEST_NAMESPACE

class MinedBlock
{
public:
    MinedBlock(const mw::Block::Ptr& pBlock, std::vector<Tx>&& txs)
        : m_pBlock(pBlock), m_txs(std::move(txs)) { }
    MinedBlock(const mw::Block::Ptr& pBlock, const std::vector<Tx>& txs)
        : m_pBlock(pBlock), m_txs(txs) { }

    const mw::Header::CPtr& GetHeader() const noexcept { return m_pBlock->GetHeader(); }
    const mw::Block::Ptr& GetBlock() const noexcept { return m_pBlock; }

private:
    mw::Block::Ptr m_pBlock;
    std::vector<Tx> m_txs;
};

END_NAMESPACE