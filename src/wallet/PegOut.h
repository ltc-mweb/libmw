#pragma once

#include <mw/models/tx/Transaction.h>
#include <mw/models/crypto/Bech32Address.h>

// Forward Declarations
class Wallet;

class PegOut
{
public:
    PegOut(const Wallet& wallet)
        : m_wallet(wallet) { }

    /// <summary>
    /// Creates a peg-out transaction.
    /// </summary>
    /// <param name="amount">The amount to peg-out.</param>
    /// <param name="amount">The fee base rate.</param>
    /// <param name="address">The canonical LTC address to send to.</param>
    /// <returns>The non-null peg-out transaction that was created.</returns>
    /// <throws>When a failure occurs in one of the underlying libraries.</throws>
    mw::Transaction::CPtr CreatePegOutTx(
        const uint64_t amount,
        const uint64_t fee_base,
        const Bech32Address& address
    ) const;

private:
    const Wallet& m_wallet;
};