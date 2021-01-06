#pragma once

#include <hash.h>
#include <mw/models/crypto/Hash.h>
#include <mw/traits/Serializable.h>

static mw::Hash Hashed(const std::vector<uint8_t>& serialized)
{
    return mw::Hash(SerializeHash(serialized).begin());
}

static mw::Hash Hashed(const Traits::ISerializable& serializable)
{
    Serializer serializer;
    serializable.Serialize(serializer);
    return mw::Hash(SerializeHash(serializer.vec()).begin());
}

static const mw::Hash& InputMessage()
{
    static const mw::Hash mweb_hash = Hashed({ 'M', 'W', 'E', 'B' }); // TODO: Determine actual message
    return mweb_hash;
}