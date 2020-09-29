#include <mw/node/CoinsView.h>

MW_NAMESPACE

void ICoinsView::ValidateMMRs(const mw::Header::CPtr& pHeader) const
{
    if (pHeader->GetKernelRoot() != GetKernelMMR()->Root()
        || pHeader->GetNumKernels() != GetKernelMMR()->GetNumLeaves()
        || pHeader->GetOutputRoot() != GetOutputPMMR()->Root()
        || pHeader->GetNumTXOs() != GetOutputPMMR()->GetNumLeaves()
        || pHeader->GetRangeProofRoot() != GetRangeProofPMMR()->Root()
        || pHeader->GetNumTXOs() != GetRangeProofPMMR()->GetNumLeaves()
        || pHeader->GetLeafsetRoot() != GetLeafSet()->Root()) {
        ThrowValidation(EConsensusError::MMR_MISMATCH);
    }
}

END_NAMESPACE