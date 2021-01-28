#pragma once

#include <hash.h>
#include <mw/models/crypto/Hash.h>
#include <mw/traits/Serializable.h>

enum class EHashTag : char
{
    ADDRESS = 'A'
};

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

// FUTURE: Incrementally update hash on each Append using CSHA256.write()
class Hasher
{
public:
    Hasher() = default;
    Hasher(const EHashTag tag)
    {
        m_serializer.Append<char>(static_cast<char>(tag));
    }

    mw::Hash hash() const { return Hashed(m_serializer.vec()); }

    template <class T>
    Hasher& Append(const T& t)
    {
        m_serializer.Append(t);
        return *this;
    }

private:
    Serializer m_serializer;
};