#pragma once

#include <mw/models/tx/Transaction.h>
#include <mw/models/wallet/StealthAddress.h>

// Forward Declarations
class Wallet;

class PegIn
{
public:
    PegIn(const Wallet& wallet)
        : m_wallet(wallet) { }

    /// <summary>
    /// Creates a peg-in transaction.
    /// </summary>
    /// <param name="amount">The amount to peg-in.</param>
    /// <param name="receiver_addr">The stealth address of the receiver (possibly self).</param>
    /// <returns>The non-null peg-in transaction that was created.</returns>
    /// <throws>When a failure occurs in one of the underlying libraries.</throws>
    mw::Transaction::CPtr CreatePegInTx(
        const uint64_t amount,
        const StealthAddress& receiver_addr
    ) const;

private:
    const Wallet& m_wallet;
};