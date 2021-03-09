#pragma once

#include <mw/models/crypto/Hash.h>
#include <boost/optional.hpp>

/// <summary>
/// Represents the state of the MMR files with the matching index.
/// </summary>
struct MMRInfo : public Traits::ISerializable
{
    MMRInfo()
        : index(0), pruned(mw::Hash()), compacted(boost::none) { }
    MMRInfo(uint32_t index_in, mw::Hash pruned_in, boost::optional<mw::Hash> compacted_in)
        : index(index_in), pruned(std::move(pruned_in)), compacted(std::move(compacted_in)) { }

    // Index of the MMR
    uint32_t index;

    // Hash of latest header this MMR represents.
    mw::Hash pruned;

    // Hash of the header this MMR was compacted for.
    // You cannot rewind beyond this point.
    boost::optional<mw::Hash> compacted;

    Serializer& Serialize(Serializer& serializer) const noexcept final
    {
        return serializer
            .Append<uint32_t>(index)
            .Append(pruned)
            .Append(compacted.value_or(mw::Hash()));
    }

    static MMRInfo Deserialize(Deserializer& deserializer)
    {
        uint32_t index = deserializer.Read<uint32_t>();
        mw::Hash pruned = deserializer.Read<mw::Hash>();
        mw::Hash compacted = deserializer.Read<mw::Hash>();

        return MMRInfo{
            index,
            pruned,
            compacted == mw::Hash() ? boost::none : boost::make_optional<mw::Hash>(std::move(compacted))
        };
    }
};