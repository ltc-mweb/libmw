#pragma once

#include <mw/models/crypto/BigInteger.h>
#include <mw/traits/Jsonable.h>
#include <mw/traits/Printable.h>
#include <mw/traits/Serializable.h>

class PublicKey :
    public Traits::IPrintable,
    public Traits::ISerializable,
    public Traits::IJsonable
{
public:
    PublicKey() = default;
    PublicKey(BigInt<33>&& compressed) : m_compressed(std::move(compressed)) {}
    PublicKey(const BigInt<33>& compressed) : m_compressed(compressed) {}
    PublicKey(const PublicKey&) = default;
    PublicKey(PublicKey&&) = default;
    virtual ~PublicKey() = default;

    //
    // Operators
    //
    PublicKey& operator=(const PublicKey& rhs) = default;
    PublicKey& operator=(PublicKey&& other) noexcept = default;
    bool operator==(const PublicKey& rhs) const { return m_compressed == rhs.m_compressed; }
    bool operator!=(const PublicKey& rhs) const { return m_compressed != rhs.m_compressed; }

    const BigInt<33>& GetBigInt() const { return m_compressed; }
    const std::vector<uint8_t>& vec() const { return m_compressed.vec(); }
    const uint8_t* data() const { return m_compressed.data(); }
    uint8_t* data() { return m_compressed.data(); }
    size_t size() const { return m_compressed.size(); }

    Serializer& Serialize(Serializer& serializer) const noexcept final { return m_compressed.Serialize(serializer); }
    static PublicKey Deserialize(Deserializer& deserializer) { return BigInt<33>::Deserialize(deserializer); }

    json ToJSON() const noexcept final
    {
        return json(m_compressed.ToHex());
    }

    static PublicKey FromJSON(const Json& json)
    {
        return PublicKey(BigInt<33>::FromHex(json.Get<std::string>()));
    }

    std::string Format() const final { return m_compressed.ToHex(); }

private:
    BigInt<33> m_compressed;
};