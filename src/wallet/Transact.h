#pragma once

#include <mw/models/tx/Transaction.h>
#include <mw/models/wallet/StealthAddress.h>
#include <mw/models/tx/PegInCoin.h>
#include <mw/models/tx/PegOutCoin.h>

// Forward Declarations
class Wallet;

class Transact
{
    struct Outputs
    {
        BlindingFactor total_blind;
        BlindingFactor total_key;
        std::vector<Output> outputs;
    };

public:
    Transact(const Wallet& wallet)
        : m_wallet(wallet) { }

    mw::Transaction::CPtr CreateTx(
        const std::vector<Commitment>& input_commits,
        const std::vector<std::pair<uint64_t, StealthAddress>>& recipients,
        const std::vector<PegOutCoin>& pegouts,
        const boost::optional<uint64_t>& pegin_amount,
        const uint64_t fee
    ) const;

private:
    Outputs CreateOutputs(const std::vector<std::pair<uint64_t, StealthAddress>>& recipients) const;

    const Wallet& m_wallet;
};