#include <libmw/defs.h>

#include <mw/models/block/Block.h>
#include <mw/models/tx/Transaction.h>
#include <mw/node/INode.h>

LIBMW_NAMESPACE

MWEXPORT libmw::HeaderRef BlockRef::GetHeader() const
{
    if (pBlock == nullptr) {
        return libmw::HeaderRef{ nullptr };
    }

    return libmw::HeaderRef{ pBlock->GetHeader() };
}

MWEXPORT uint64_t BlockRef::GetTotalFee() const noexcept
{
    assert(pBlock != nullptr);
    return pBlock->GetTotalFee();
}

MWEXPORT std::set<KernelHash> BlockRef::GetKernelHashes() const
{
    assert(pBlock != nullptr);
    std::set<KernelHash> kernelHashes;
    for (const Kernel& kernel : pBlock->GetKernels()) {
        kernelHashes.insert(kernel.GetHash().ToArray());
    }
    return kernelHashes;
}

MWEXPORT std::vector<PegOut> TxRef::GetPegouts() const noexcept
{
    std::vector<PegOut> pegouts;
    for (const Kernel& kernel : pTransaction->GetKernels()) {
        if (kernel.IsPegOut()) {
            PegOut pegout;
            pegout.amount = kernel.GetPeggedOut();
            pegout.address = kernel.GetAddress().value().ToString();
            pegouts.emplace_back(std::move(pegout));
        }
    }

    return pegouts;
}

MWEXPORT uint64_t TxRef::GetTotalFee() const noexcept
{
    assert(pTransaction != nullptr);
    return pTransaction->GetTotalFee();
}

MWEXPORT std::set<KernelHash> TxRef::GetKernelHashes() const
{
    assert(pTransaction != nullptr);
    std::set<KernelHash> kernelHashes;
    for (const Kernel& kernel : pTransaction->GetKernels()) {
        kernelHashes.insert(kernel.GetHash().ToArray());
    }
    return kernelHashes;
}

MWEXPORT libmw::CoinsViewRef CoinsViewRef::CreateCache() const
{
    if (pCoinsView == nullptr) {
        return libmw::CoinsViewRef{ nullptr };
    }

    return libmw::CoinsViewRef{ std::make_shared<mw::CoinsViewCache>(pCoinsView) };
}

END_NAMESPACE