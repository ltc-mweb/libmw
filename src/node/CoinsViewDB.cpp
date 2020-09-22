#include <mw/node/CoinsView.h>

#include "models/CoinKey.h"
#include "models/CoinEntry.h"

#include <mw/exceptions/ValidationException.h>

MW_NAMESPACE

void CoinsViewDB::ApplyUpdates(const TxBody& body)
{
    auto pBatch = m_pDatabase->CreateBatch();
    WriteBatch(*pBatch, body);
}

std::vector<UTXO::CPtr> CoinsViewDB::GetUTXOs(const Commitment& commitment) const
{
    std::vector<uint8_t> value;
    m_pDatabase->Read(CoinKey(commitment).ToString(), value); // TODO: Need to read from batch first
    return CoinEntry::Deserialize(value).utxos;
}

void CoinsViewDB::AddUTXO(mw::db::IDBBatch& batch, const Output& output)
{
    mmr::LeafIndex leafIdx = m_pOutputPMMR->Add(OutputId{ output.GetFeatures(), output.GetCommitment() });
    mmr::LeafIndex leafIdx2 = m_pRangeProofPMMR->Add(*output.GetRangeProof());
    assert(leafIdx == leafIdx2);

    m_pLeafSet->Add(leafIdx);

    AddUTXO(batch, std::make_shared<UTXO>(GetBestHeader()->GetHeight(), std::move(leafIdx), output));
}

void CoinsViewDB::AddUTXO(mw::db::IDBBatch& batch, const UTXO::CPtr& pUTXO)
{
    const Commitment& commitment = pUTXO->GetOutput().GetCommitment();
    std::vector<UTXO::CPtr> utxos = GetUTXOs(commitment);
    utxos.push_back(pUTXO);

    batch.Write(CoinKey{ commitment }.ToString(), CoinEntry{ std::move(utxos) }.Serialized());
}

void CoinsViewDB::SpendUTXO(mw::db::IDBBatch& batch, const Commitment& commitment)
{
    std::vector<UTXO::CPtr> utxos = GetUTXOs(commitment);
    if (utxos.empty()) {
		ThrowValidation(EConsensusError::UTXO_MISSING);
    }

    m_pLeafSet->Remove(utxos.back()->GetLeafIndex());

    utxos.pop_back();
    batch.Write(CoinKey{ commitment }.ToString(), CoinEntry{ std::move(utxos) }.Serialized());
}

void CoinsViewDB::WriteBatch(mw::db::IDBBatch& batch, const TxBody& body)
{
    std::for_each(
        body.GetKernels().cbegin(), body.GetKernels().cend(),
        [this](const Kernel& kernel) { m_pKernelMMR->Add(kernel); }
    );

    std::for_each(
        body.GetInputs().cbegin(), body.GetInputs().cend(),
        [this, &batch](const Input& input) { SpendUTXO(batch, input.GetCommitment()); }
    );

    std::for_each(
        body.GetOutputs().cbegin(), body.GetOutputs().cend(),
        [this, &batch](const Output& output) { AddUTXO(batch, output); }
    );
}

END_NAMESPACE