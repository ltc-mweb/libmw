#pragma once

#include <mw/common/Macros.h>
#include <mw/models/tx/Transaction.h>
#include <mw/models/tx/PegInCoin.h>
#include <mw/node/CoinsView.h>
#include <memory>

MW_NAMESPACE

class BlockBuilder
{
public:
    using Ptr = std::shared_ptr<BlockBuilder>;

    BlockBuilder(const uint64_t height, const mw::ICoinsView::Ptr& pCoinsView)
        : m_height(height), m_pCoinsView(std::make_shared<mw::CoinsViewCache>(pCoinsView)) { }

    bool AddTransaction(const Transaction::CPtr& pTransaction, const std::vector<PegInCoin>& pegins);

    mw::Block::Ptr BuildBlock() const;

private:
    uint64_t m_height;
    mw::CoinsViewCache::Ptr m_pCoinsView;
    std::vector<Transaction::CPtr> m_transactions;
};

END_NAMESPACE // mw