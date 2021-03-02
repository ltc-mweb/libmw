#include <mw/node/BlockBuilder.h>
#include <mw/consensus/KernelSumValidator.h>
#include <unordered_set>

MW_NAMESPACE

bool BlockBuilder::AddTransaction(const Transaction::CPtr& pTransaction, const std::vector<PegInCoin>& pegins)
{
    // TODO - Check weight

    // Verify pegin amount matches
    const uint64_t actual_amount = pTransaction->GetPegInAmount();
    const uint64_t expected_amount = std::accumulate(pegins.cbegin(), pegins.cend(), (uint64_t)0,
        [](const uint64_t sum, const PegInCoin& pegin) { return sum + pegin.GetAmount(); }
    );
    if (actual_amount != expected_amount) {
        std::cout << "Mismatched pegin amount" << std::endl;
        return false;
    }

    // Verify pegin commitments are unique
    std::unordered_set<Commitment> pegin_commitments;
    for (const PegInCoin& pegin : pegins) {
        if (pegin_commitments.find(pegin.GetCommitment()) != pegin_commitments.end()) {
            std::cout << "Duplicate pegin commitments" << std::endl;
            return false;
        }

        pegin_commitments.insert(pegin.GetCommitment());
    }

    // Verify pegin outputs are included
    std::vector<PegInCoin> pegin_coins = pTransaction->GetPegIns();
    if (pegin_coins.size() != pegins.size()) {
        std::cout << "Mismatched pegin count" << std::endl;
        return false;
    }

    for (const PegInCoin& pegin : pegin_coins) {
        if (pegin_commitments.find(pegin.GetCommitment()) == pegin_commitments.end()) {
            std::cout << "Pegin commitment not found" << std::endl;
            return false;
        }
    }

    // Validate transaction
    pTransaction->Validate();

    // Add transaction
    mw::CoinsViewCache cache(m_pCoinsView);
    std::vector<Transaction::CPtr> transactions = m_transactions;
    transactions.push_back(pTransaction);

    auto pBlock = cache.BuildNextBlock(m_height, transactions);
    assert(pBlock != nullptr);

    m_transactions.push_back(pTransaction);
    return true;
}

mw::Block::Ptr BlockBuilder::BuildBlock() const
{
    mw::CoinsViewCache cache(m_pCoinsView);
    return cache.BuildNextBlock(m_height, m_transactions);
}

END_NAMESPACE