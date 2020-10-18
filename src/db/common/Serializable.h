#include <mw/serialization/Serializer.h>

class SerializableInt : public Traits::ISerializable
{
public:
    SerializableInt(uint64_t value) : m_value(value) {}

    uint64_t Get() const { return m_value; }

    Serializer& Serialize(Serializer& serializer) const noexcept final
    {
        return serializer.Append(m_value);
    }

    static SerializableInt Deserialize(Deserializer& deserializer)
    {
        return SerializableInt(deserializer.Read<uint64_t>());
    }

private:
    uint64_t m_value;
};

class SerializableVec : public Traits::ISerializable
{
public:
    SerializableVec(std::vector<uint8_t>&& bytes) : m_bytes(std::move(bytes)) {}

    std::vector<uint8_t> Get() const { return m_bytes; }

    Serializer& Serialize(Serializer& serializer) const noexcept final
    {
        return serializer.Append(m_bytes);
    }

    static SerializableVec Deserialize(Deserializer& deserializer)
    {
        return SerializableVec(deserializer.ReadVector(deserializer.GetRemainingSize()));
    }

private:
    std::vector<uint8_t> m_bytes;
};