#pragma once

#include <mw/crypto/Keys.h>
#include <mw/models/crypto/PublicKey.h>
#include <mw/models/crypto/Bech32Address.h>
#include <mw/traits/Serializable.h>

class StealthAddress : public Traits::ISerializable
{
public:
    StealthAddress(PublicKey&& scan, PublicKey&& spend)
        : m_scan(std::move(scan)), m_spend(std::move(spend)) { }
    StealthAddress(const PublicKey& scan, const PublicKey& spend)
        : m_scan(scan), m_spend(spend) { }

    bool operator==(const StealthAddress& rhs) const noexcept
    {
        return m_scan == rhs.m_scan && m_spend == rhs.m_spend;
    }

    static StealthAddress Random()
    {
        return StealthAddress(Keys::Random().PubKey(), Keys::Random().PubKey());
    }

    static StealthAddress Decode(const std::string& encoded)
    {
        std::vector<std::string> parts = StringUtil::Split(encoded, ":");
        Bech32Address first_addr = Bech32Address::FromString(parts[0]);
        Bech32Address second_addr = Bech32Address::FromString(parts[1]);

        return StealthAddress(
            PublicKey{ first_addr.GetAddress() },
            PublicKey{ second_addr.GetAddress() }
        );
    }

    std::string Encode() const
    {
        Bech32Address first_addr("mweb", m_scan.vec());
        Bech32Address second_addr("ltc", m_spend.vec());
        return first_addr.ToString() + ":" + second_addr.ToString();
    }

    const PublicKey& A() const noexcept { return m_scan; }
    const PublicKey& B() const noexcept { return m_spend; }

    const PublicKey& GetScanPubKey() const noexcept { return m_scan; }
    const PublicKey& GetSpendPubKey() const noexcept { return m_spend; }

    Serializer& Serialize(Serializer& serializer) const noexcept final
    {
        return serializer.Append(m_scan).Append(m_spend);
    }

    static StealthAddress Deserialize(Deserializer& deserializer)
    {
        PublicKey scan = PublicKey::Deserialize(deserializer);
        PublicKey spend = PublicKey::Deserialize(deserializer);
        return StealthAddress(std::move(scan), std::move(spend));
    }

private:
    PublicKey m_scan;
    PublicKey m_spend;
};