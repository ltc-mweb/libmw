#pragma once

#include "Transformers.h"

#include <mw/db/IBlockStore.h>

class BlockStoreWrapper : public mw::IBlockStore
{
public:
    BlockStoreWrapper(const mw::db::IBlockStore* pBlockStore)
        : m_pBlockStore(pBlockStore) { }

    mw::Header::CPtr GetHeader(const uint64_t height) const final { return m_pBlockStore->GetHeader(height).pHeader; }
    mw::Header::CPtr GetHeader(const mw::Hash& hash) const final { return m_pBlockStore->GetHeader(hash.ToArray()).pHeader; }

    mw::HeaderAndPegs::CPtr GetHeaderAndPegs(const uint64_t height) const final { return Transform(m_pBlockStore->GetHeaderAndPegs(height)); }
    mw::HeaderAndPegs::CPtr GetHeaderAndPegs(const mw::Hash& hash) const final { return Transform(m_pBlockStore->GetHeaderAndPegs(hash.ToArray())); }

    mw::Block::CPtr GetBlock(const uint64_t height) const final { return m_pBlockStore->GetBlock(height).pBlock; }
    mw::Block::CPtr GetBlock(const mw::Hash& hash) const final { return m_pBlockStore->GetBlock(hash.ToArray()).pBlock; }

private:
    mw::HeaderAndPegs::CPtr Transform(const mw::HeaderAndPegsRef& ref) const
    {
        auto pegins = TransformPegIns(ref.pegins);
        auto pegouts = TransformPegOuts(ref.pegouts);

        return std::make_shared<mw::HeaderAndPegs>(ref.header.pHeader, std::move(pegins), std::move(pegouts));
    }

    const mw::db::IBlockStore* m_pBlockStore;
};