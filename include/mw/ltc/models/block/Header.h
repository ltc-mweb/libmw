#pragma once

#include <mw/core/models/block/IHeader.h>

class Header : public IHeader
{
public:
    using CPtr = std::shared_ptr<const Header>;

    Header(
        const uint64_t height,
        Hash&& previousHash,
        Hash&& outputRoot,
        Hash&& rangeProofRoot,
        Hash&& kernelRoot,
        BlindingFactor&& offset,
        const uint64_t outputMMRSize,
        const uint64_t kernelMMRSize
    )
        : IHeader(
            height,
            std::move(previousHash),
            std::move(outputRoot),
            std::move(rangeProofRoot),
            std::move(kernelRoot),
            std::move(offset),
            outputMMRSize,
            kernelMMRSize)
    {

    }

    //
    // Serialization/Deserialization
    //
    Serializer& Serialize(Serializer& serializer) const noexcept final
    {
        return serializer
            .Append<uint64_t>(m_height)
            .Append(m_previousHash)
            .Append(m_outputRoot)
            .Append(m_rangeProofRoot)
            .Append(m_kernelRoot)
            .Append(m_offset)
            .Append<uint64_t>(m_outputMMRSize)
            .Append<uint64_t>(m_kernelMMRSize);
    }

    static Header::CPtr Deserialize(Deserializer& deserializer)
    {
        uint64_t height = deserializer.Read<uint64_t>();
        Hash previousHash = Hash::Deserialize(deserializer);
        Hash outputRoot = Hash::Deserialize(deserializer);
        Hash proofRoot = Hash::Deserialize(deserializer);
        Hash kernelRoot = Hash::Deserialize(deserializer);
        BlindingFactor offset = BlindingFactor::Deserialize(deserializer);
        uint64_t outputMMRSize = deserializer.Read<uint64_t>();
        uint64_t kernelMMRSize = deserializer.Read<uint64_t>();

        return std::make_shared<Header>(
            height,
            std::move(previousHash),
            std::move(outputRoot),
            std::move(proofRoot),
            std::move(kernelRoot),
            std::move(offset),
            outputMMRSize,
            kernelMMRSize
        );
    }

    json ToJSON() const noexcept final
    {
        // TODO: Implement
        return json();
    }

    static Header::CPtr FromJSON(const Json& json)
    {
        // TODO: Implement
        return nullptr;
    }

    void Validate(const Context& context) const final
    {

    }
};