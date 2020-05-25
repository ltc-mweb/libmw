#pragma once

#include <mw/core/models/block/IBlock.h>
#include <mw/ltc/models/block/Header.h>
#include <mw/ltc/models/tx/Kernel.h>
#include <mw/ltc/models/tx/PegInCoin.h>
#include <mw/ltc/models/tx/PegOutCoin.h>
#include <algorithm>

class Block : public IBlock
{
public:
    using Ptr = std::shared_ptr<Block>;
    using CPtr = std::shared_ptr<const Block>;

    //
    // Constructors
    //
    Block(const Header::CPtr& pHeader, TxBody&& body)
        : IBlock(pHeader, std::move(body)), m_validated(false) { }
    Block(const Block& other) = default;
    Block(Block&& other) noexcept = default;
    Block() = default;

    //
    // Operators
    //
    Block& operator=(const Block& other) = default;
    Block& operator=(Block&& other) noexcept = default;

    std::vector<Kernel::CPtr> GetAllKernels() const noexcept
    {
        const auto& kernels = GetKernels();

        std::vector<Kernel::CPtr> transformed;
        std::transform(
            kernels.cbegin(), kernels.cend(),
            std::back_inserter(transformed),
            [](const auto& pKernel) -> Kernel::CPtr { return std::dynamic_pointer_cast<const Kernel>(pKernel); }
        );

        return transformed;
    }

    std::vector<Kernel::CPtr> GetPegInKernels() const noexcept
    {
        const auto& kernels = GetAllKernels();

        std::vector<Kernel::CPtr> peggedIn;
        std::copy_if(
            kernels.cbegin(), kernels.cend(),
            std::back_inserter(peggedIn),
            [](const auto& pKernel) -> bool { return pKernel->IsPegIn(); }
        );

        return peggedIn;
    }

    uint64_t GetPegInAmount() const noexcept
    {
        const auto kernels = GetAllKernels();
        return std::accumulate(
            kernels.cbegin(), kernels.cend(), (uint64_t)0,
            [](const uint64_t sum, const auto& pKernel) noexcept { return sum + pKernel->GetPeggedIn(); }
        );
    }

    std::vector<Kernel::CPtr> GetPegOutKernels() const noexcept
    {
        const auto kernels = GetAllKernels();

        std::vector<Kernel::CPtr> peggedOut;
        std::copy_if(
            kernels.cbegin(), kernels.cend(),
            std::back_inserter(peggedOut),
            [](const auto& pKernel) -> bool { return pKernel->IsPegOut(); }
        );

        return peggedOut;
    }

    uint64_t GetTotalFee() const noexcept
    {
        const auto& kernels = GetKernels();
        return std::accumulate(
            kernels.cbegin(), kernels.cend(), (uint64_t)0,
            [](const uint64_t sum, const auto& pKernel) noexcept { return sum + pKernel->GetFee(); }
        );
    }

    void Validate(const Context::CPtr& pContext) const final
    {

    }

    bool WasValidated() const noexcept { return m_validated; }
    void MarkAsValidated() noexcept { m_validated = true; }

private:
    bool m_validated;
};