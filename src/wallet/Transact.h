#pragma once

#include <mw/models/tx/Transaction.h>
#include <mw/models/wallet/StealthAddress.h>

// Forward Declarations
class Wallet;

class Transact
{
public:
    Transact(const Wallet& wallet)
        : m_wallet(wallet) { }

    /// <summary>
    /// Creates a standard MWEB-to-MWEB transaction.
    /// </summary>
    /// <param name="amount">The amount to send.</param>
    /// <param name="fee_base">The fee base rate.</param>
    /// <param name="receiver_addr">The stealth address of the receiver.</param>
    /// <returns>The non-null MWEB-to-MWEB transaction that was created.</returns>
    /// <throws>When a failure occurs in one of the underlying libraries.</throws>
    mw::Transaction::CPtr CreateTx(
        const uint64_t amount,
        const uint64_t fee_base,
        const StealthAddress& receiver_addr
    ) const;

private:
    const Wallet& m_wallet;
};