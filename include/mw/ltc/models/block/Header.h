#pragma once

#include <mw/core/models/block/Header.h>

class Header : public IHeader
{
public:
    Header(
        const uint16_t version,
        const uint64_t height,
        Hash&& previousHash,
        Hash&& previousRoot,
        Hash&& outputRoot,
        Hash&& rangeProofRoot,
        Hash&& kernelRoot,
        BlindingFactor&& offset,
        const uint64_t outputMMRSize,
        const uint64_t kernelMMRSize
    )
        : IHeader(
            version,
            height,
            std::move(previousHash),
            std::move(previousRoot),
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
    virtual Serializer& Serialize(Serializer& serializer) const override final
    {
        return serializer
            .Append<uint16_t>(m_version)
            .Append<uint64_t>(m_height)
            .Append(m_previousHash)
            .Append(m_previousRoot)
            .Append(m_outputRoot)
            .Append(m_rangeProofRoot)
            .Append(m_kernelRoot)
            .Append(m_offset)
            .Append<uint64_t>(m_outputMMRSize)
            .Append<uint64_t>(m_kernelMMRSize);
    }

    static IHeader::CPtr Deserialize(ByteBuffer& byteBuffer)
    {
        uint16_t version = byteBuffer.ReadU16();
        uint64_t height = byteBuffer.ReadU64();
        Hash previousHash = Hash::Deserialize(byteBuffer);
        Hash previousRoot = Hash::Deserialize(byteBuffer);
        Hash outputRoot = Hash::Deserialize(byteBuffer);
        Hash proofRoot = Hash::Deserialize(byteBuffer);
        Hash kernelRoot = Hash::Deserialize(byteBuffer);
        BlindingFactor offset = BlindingFactor::Deserialize(byteBuffer);
        uint64_t outputMMRSize = byteBuffer.ReadU64();
        uint64_t kernelMMRSize = byteBuffer.ReadU64();
        return std::make_shared<Header>(
            version,
            height,
            std::move(previousHash),
            std::move(previousRoot),
            std::move(outputRoot),
            std::move(proofRoot),
            std::move(kernelRoot),
            std::move(offset),
            outputMMRSize,
            kernelMMRSize
        );
    }

    virtual void Validate(const NodeContext& context) const override final
    {

    }
};