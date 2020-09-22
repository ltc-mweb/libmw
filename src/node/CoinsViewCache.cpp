#include <mw/node/CoinsView.h>
#include <mw/exceptions/ValidationException.h>

MW_NAMESPACE

std::vector<UTXO::CPtr> CoinsViewCache::GetUTXOs(const Commitment& commitment) const
{
    std::vector<UTXO::CPtr> utxos = m_pBase->GetUTXOs(commitment);

    const std::vector<CoinAction>& actions = m_pUpdates->GetActions(commitment);
    for (const CoinAction& action : actions) {
        if (action.pUTXO != nullptr) {
            utxos.push_back(action.pUTXO);
        } else {
            assert(!utxos.empty());
            utxos.pop_back();
        }
    }

    //auto iter = m_actions.find(commitment);
    //if (iter != m_actions.cend()) {
    //    const std::vector<Action>& actions = iter->second;
    //    for (const Action& action : actions) {
    //        if (action.pUTXO != nullptr) {
    //            utxos.push_back(action.pUTXO);
    //        } else {
    //            assert(!utxos.empty());
    //            utxos.pop_back();
    //        }
    //    }
    //}

    return utxos;
}

void CoinsViewCache::ApplyUpdates(const TxBody& body)
{
    std::for_each(
        body.GetKernels().cbegin(), body.GetKernels().cend(),
        [this](const Kernel& kernel) { m_pKernelMMR->Add(kernel); }
    );

    std::for_each(
        body.GetInputs().cbegin(), body.GetInputs().cend(),
        [this](const Input& input) { SpendUTXO(input.GetCommitment()); }
    );

    std::for_each(
        body.GetOutputs().cbegin(), body.GetOutputs().cend(),
        [this](const Output& output) { AddUTXO(output); }
    );
}

void CoinsViewCache::AddUTXO(const Output& output)
{
    mmr::LeafIndex leafIdx = m_pOutputPMMR->Add(OutputId{ output.GetFeatures(), output.GetCommitment() });
    mmr::LeafIndex leafIdx2 = m_pRangeProofPMMR->Add(*output.GetRangeProof());
    assert(leafIdx == leafIdx2);

    m_pLeafSet->Add(leafIdx);

    auto pUTXO = std::make_shared<UTXO>(GetBestHeader()->GetHeight(), std::move(leafIdx), output);

    m_pUpdates->AddUTXO(pUTXO);
    //auto iter = m_actions.find(output.GetCommitment());
    //if (iter != m_actions.end()) {
    //    std::vector<Action>& actions = iter->second;
    //    actions.push_back({ pUTXO });
    //} else {
    //    std::vector<Action> actions;
    //    actions.push_back({ pUTXO });
    //    m_actions.insert({ output.GetCommitment(), actions });
    //}
}

void CoinsViewCache::SpendUTXO(const Commitment& commitment)
{
    std::vector<UTXO::CPtr> utxos = GetUTXOs(commitment);
    if (utxos.empty()) {
        ThrowValidation(EConsensusError::UTXO_MISSING);
    }

    m_pLeafSet->Remove(utxos.back()->GetLeafIndex());

    m_pUpdates->SpendUTXO(commitment);
    //auto iter = m_actions.find(commitment);
    //if (iter != m_actions.end()) {
    //    std::vector<Action>& actions = iter->second;
    //    actions.push_back({ nullptr });
    //}
    //else {
    //    std::vector<Action> actions;
    //    actions.push_back({ nullptr });
    //    m_actions.insert({ commitment, actions });
    //}
}

void CoinsViewCache::Flush(mw::db::IDBBatch& batch)
{
    // TODO: Implement
}

END_NAMESPACE