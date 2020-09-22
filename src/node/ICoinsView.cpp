#include <mw/node/CoinsView.h>
#include <mw/consensus/Aggregation.h>
#include <mw/common/Logger.h>

MW_NAMESPACE

mw::Block::CPtr ICoinsView::BuildNextBlock(const std::vector<mw::Transaction::CPtr>& transactions) const
{
    LOG_TRACE_F("Building block with {} transactions", transactions.size());
    auto pTransaction = Aggregation::Aggregate(transactions);

    CoinsViewCache temp_cache(shared_from_this());
    temp_cache.ApplyUpdates(pTransaction->GetBody());

    const uint64_t output_mmr_size = temp_cache.GetOutputPMMR()->GetNumLeaves();
    const uint64_t kernel_mmr_size = temp_cache.GetKernelMMR()->GetNumLeaves();

    mw::Hash output_root = temp_cache.GetOutputPMMR()->Root();
    mw::Hash rangeproof_root = temp_cache.GetRangeProofPMMR()->Root();
    mw::Hash kernel_root = temp_cache.GetKernelMMR()->Root();
    mw::Hash leafset_root = temp_cache.GetLeafSet()->Root(output_mmr_size);

    const uint64_t height = GetBestHeader()->GetHeight() + 1;
    BlindingFactor total_offset = Crypto::AddBlindingFactors({ GetBestHeader()->GetOffset(), pTransaction->GetOffset() });

    auto pHeader = std::make_shared<mw::Header>(
        height,
        std::move(output_root),
        std::move(rangeproof_root),
        std::move(kernel_root),
        std::move(leafset_root),
        std::move(total_offset),
        output_mmr_size,
        kernel_mmr_size
    );

    return std::make_shared<mw::Block>(pHeader, pTransaction->GetBody());
}

END_NAMESPACE