#pragma once

#include <mw/crypto/Keys.h>
#include <mw/models/crypto/PublicKey.h>

class StealthAddress
{
public:
    StealthAddress(const PublicKey& scan, const PublicKey& spend)
        : m_scan(scan), m_spend(spend) { }

    static StealthAddress Random()
    {
        return StealthAddress(Keys::Random().PubKey(), Keys::Random().PubKey());
    }

    const PublicKey& A() const noexcept { return m_scan; }
    const PublicKey& B() const noexcept { return m_spend; }

    const PublicKey& GetScanPubKey() const noexcept { return m_scan; }
    const PublicKey& GetSpendPubKey() const noexcept { return m_spend; }

private:
    PublicKey m_scan;
    PublicKey m_spend;
};