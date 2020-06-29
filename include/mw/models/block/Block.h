#pragma once

#include <mw/common/Macros.h>
#include <mw/models/tx/TxBody.h>
#include <mw/models/block/Header.h>
#include <mw/models/tx/Kernel.h>
#include <mw/models/tx/PegInCoin.h>
#include <mw/models/tx/PegOutCoin.h>
#include <mw/traits/Hashable.h>
#include <mw/traits/Serializable.h>
#include <mw/traits/Printable.h>
#include <mw/traits/Jsonable.h>
#include <algorithm>

MW_NAMESPACE

// TODO: This should contain peg-in and peg-out info.
class Block final :
    public Traits::IPrintable,
    public Traits::ISerializable,
    public Traits::IHashable,
    public Traits::IJsonable
{
public:
    using Ptr = std::shared_ptr<Block>;
    using CPtr = std::shared_ptr<const Block>;

    //
    // Constructors
    //
    Block(const mw::Header::CPtr& pHeader, TxBody&& body)
        : m_pHeader(pHeader), m_body(std::move(body)), m_validated(false) { }
    Block(const mw::Header::CPtr& pHeader, const TxBody& body)
        : m_pHeader(pHeader), m_body(body), m_validated(false) { }
    Block(const Block& other) = default;
    Block(Block&& other) noexcept = default;
    Block() = default;

    //
    // Operators
    //
    Block& operator=(const Block& other) = default;
    Block& operator=(Block&& other) noexcept = default;

    //
    // Getters
    //
    const mw::Header::CPtr& GetHeader() const noexcept { return m_pHeader; }
    const TxBody& GetTxBody() const noexcept { return m_body; }

    const std::vector<Input>& GetInputs() const noexcept { return m_body.GetInputs(); }
    const std::vector<Output>& GetOutputs() const noexcept { return m_body.GetOutputs(); }
    const std::vector<Kernel>& GetKernels() const noexcept { return m_body.GetKernels(); }

    uint64_t GetHeight() const noexcept { return m_pHeader->GetHeight(); }
    const BlindingFactor& GetOffset() const noexcept { return m_pHeader->GetOffset(); }

    std::vector<Kernel> GetPegInKernels() const noexcept
    {
        const auto& kernels = GetKernels();

        std::vector<Kernel> peggedIn;
        std::copy_if(
            kernels.cbegin(), kernels.cend(),
            std::back_inserter(peggedIn),
            [](const auto& kernel) -> bool { return kernel.IsPegIn(); }
        );

        return peggedIn;
    }

    uint64_t GetPegInAmount() const noexcept
    {
        const auto kernels = GetKernels();
        return std::accumulate(
            kernels.cbegin(), kernels.cend(), (uint64_t)0,
            [](const uint64_t sum, const auto& kernel) noexcept { return sum + kernel.GetPeggedIn(); }
        );
    }

    std::vector<Kernel> GetPegOutKernels() const noexcept
    {
        const auto kernels = GetKernels();

        std::vector<Kernel> peggedOut;
        std::copy_if(
            kernels.cbegin(), kernels.cend(),
            std::back_inserter(peggedOut),
            [](const auto& kernel) -> bool { return kernel.IsPegOut(); }
        );

        return peggedOut;
    }

    uint64_t GetTotalFee() const noexcept
    {
        const auto& kernels = GetKernels();
        return std::accumulate(
            kernels.cbegin(), kernels.cend(), (uint64_t)0,
            [](const uint64_t sum, const auto& kernel) noexcept { return sum + kernel.GetFee(); }
        );
    }

    //
    // Serialization/Deserialization
    //
    Serializer& Serialize(Serializer& serializer) const noexcept final
    {
        return serializer.Append(m_pHeader).Append(m_body);
    }

    static Block Deserialize(Deserializer& deserializer)
    {
        mw::Header::CPtr pHeader = std::make_shared<mw::Header>(mw::Header::Deserialize(deserializer));
        TxBody body = deserializer.Read<TxBody>();
        return Block{ pHeader, std::move(body) };
    }

    json ToJSON() const noexcept final
    {
        return json({
            { "header", m_pHeader->ToJSON() },
            { "body", m_body }
        });
    }

    static Block FromJSON(const Json& json)
    {
        return Block{
            std::make_shared<mw::Header>(json.GetRequired<mw::Header>("header")),
            json.GetRequired<TxBody>("body")
        };
    }

    //
    // Traits
    //
    mw::Hash GetHash() const noexcept final { return m_pHeader->GetHash(); }
    std::string Format() const final { return GetHash().ToHex(); }

    //
    // Context-free validation of the block.
    //
    void Validate() const
    {
        m_pHeader->Validate();
        m_body.Validate();
    }

    bool WasValidated() const noexcept { return m_validated; }
    void MarkAsValidated() noexcept { m_validated = true; }

private:
    mw::Header::CPtr m_pHeader;
    TxBody m_body;
    bool m_validated;
};

END_NAMESPACE