#pragma once

#include <mw/common/Macros.h>
#include <mw/models/block/Header.h>
#include <mw/models/block/Block.h>
#include <mw/models/tx/Transaction.h>
#include <mw/models/tx/UTXO.h>
#include <mw/mmr/MMR.h>
#include <mw/mmr/LeafSet.h>
#include <ext/interfaces.h>
#include <memory>

MW_NAMESPACE

struct CoinAction
{
    UTXO::CPtr pUTXO;
};

class CoinsViewUpdates
{
public:
    using Ptr = std::shared_ptr<CoinsViewUpdates>;

    CoinsViewUpdates() = default;

    void AddUTXO(const UTXO::CPtr& pUTXO)
    {
        AddAction(pUTXO->GetCommitment(), CoinAction{ pUTXO });
    }

    void SpendUTXO(const Commitment& commitment)
    {
        AddAction(commitment, CoinAction{ nullptr });
    }

    std::vector<CoinAction> GetActions(const Commitment& commitment) const
    {
        auto iter = m_actions.find(commitment);
        if (iter != m_actions.cend()) {
            return iter->second;
        }

        return {};
    }

private:
    void AddAction(const Commitment commitment, CoinAction&& action)
    {
        auto iter = m_actions.find(commitment);
        if (iter != m_actions.end()) {
            std::vector<CoinAction>& actions = iter->second;
            actions.emplace_back(std::move(action));
        } else {
            std::vector<CoinAction> actions;
            actions.emplace_back(std::move(action));
            m_actions.insert({ commitment, actions });
        }
    }

    std::unordered_map<Commitment, std::vector<CoinAction>> m_actions;
};

//
// An interface for the various views of the extension block's UTXO set.
// This is similar to CCoinsView in the main codebase, and in fact, each CCoinsView
// should also hold an instance of a mw::ICoinsView for use with mimblewimble-related logic.
//
class ICoinsView : public std::enable_shared_from_this<ICoinsView>
{
public:
    using Ptr = std::shared_ptr<ICoinsView>;
    using CPtr = std::shared_ptr<const ICoinsView>;
    virtual ~ICoinsView() = default;

    mw::Block::CPtr BuildNextBlock(const std::vector<mw::Transaction::CPtr>& transactions) const;

    void SetBestHeader(const mw::Header::CPtr & pHeader) noexcept { m_pHeader = pHeader; }
    mw::Header::CPtr GetBestHeader() const noexcept { return m_pHeader; }

    // Virtual functions
    virtual std::vector<UTXO::CPtr> GetUTXOs(const Commitment& commitment) const = 0;
    virtual void ApplyUpdates(const TxBody& body) = 0;

    virtual mmr::ILeafSet::Ptr GetLeafSet() const noexcept = 0;
    virtual mmr::IMMR::Ptr GetKernelMMR() const noexcept = 0;
    virtual mmr::IMMR::Ptr GetOutputPMMR() const noexcept = 0;
    virtual mmr::IMMR::Ptr GetRangeProofPMMR() const noexcept = 0;

private:
    mw::Header::CPtr m_pHeader;
};

class CoinsViewCache : public mw::ICoinsView
{
public:
    CoinsViewCache(const ICoinsView::CPtr& pBase)
        : m_pBase(pBase),
        m_pLeafSet(std::make_shared<mmr::BackedLeafSet>(pBase->GetLeafSet())),
        m_pKernelMMR(std::make_shared<mmr::MMRCache>(pBase->GetKernelMMR())),
        m_pOutputPMMR(std::make_shared<mmr::MMRCache>(pBase->GetOutputPMMR())),
        m_pRangeProofPMMR(std::make_shared<mmr::MMRCache>(pBase->GetRangeProofPMMR())),
        m_pUpdates(std::make_shared<CoinsViewUpdates>()) { }

    std::vector<UTXO::CPtr> GetUTXOs(const Commitment& commitment) const final;
    void ApplyUpdates(const TxBody& body) final;
    void Flush(mw::db::IDBBatch& batch);

    mmr::ILeafSet::Ptr GetLeafSet() const noexcept final { return m_pLeafSet; }
    mmr::IMMR::Ptr GetKernelMMR() const noexcept final { return m_pKernelMMR; }
    mmr::IMMR::Ptr GetOutputPMMR() const noexcept final { return m_pOutputPMMR; }
    mmr::IMMR::Ptr GetRangeProofPMMR() const noexcept final { return m_pRangeProofPMMR; }

private:
    void AddUTXO(const Output& output);
    void SpendUTXO(const Commitment& commitment);

    ICoinsView::CPtr m_pBase;

    mmr::BackedLeafSet::Ptr m_pLeafSet;
    mmr::MMRCache::Ptr m_pKernelMMR;
    mmr::MMRCache::Ptr m_pOutputPMMR;
    mmr::MMRCache::Ptr m_pRangeProofPMMR;

    CoinsViewUpdates::Ptr m_pUpdates;
    //std::unordered_map<Commitment, std::vector<Action>> m_actions;
};

class CoinsViewDB : public mw::ICoinsView
{
public:
    CoinsViewDB(
        const std::shared_ptr<mw::db::IDBWrapper>& pDBWrapper,
        const mmr::LeafSet::Ptr& pLeafSet,
        const mmr::MMR::Ptr& pKernelMMR,
        const mmr::MMR::Ptr& pOutputPMMR,
        const mmr::MMR::Ptr& pRangeProofPMMR
    ) : m_pDatabase(pDBWrapper),
        m_pLeafSet(pLeafSet),
        m_pKernelMMR(pKernelMMR),
        m_pOutputPMMR(pOutputPMMR),
        m_pRangeProofPMMR(pRangeProofPMMR) { }

    std::vector<UTXO::CPtr> GetUTXOs(const Commitment& commitment) const final;
    void ApplyUpdates(const TxBody& body) final;
    void WriteBatch(mw::db::IDBBatch& batch, const TxBody& body);

    mmr::ILeafSet::Ptr GetLeafSet() const noexcept final { return m_pLeafSet; }
    mmr::IMMR::Ptr GetKernelMMR() const noexcept final { return m_pKernelMMR; }
    mmr::IMMR::Ptr GetOutputPMMR() const noexcept final { return m_pOutputPMMR; }
    mmr::IMMR::Ptr GetRangeProofPMMR() const noexcept final { return m_pRangeProofPMMR; }

private:
    void AddUTXO(mw::db::IDBBatch& batch, const Output& output);
    void AddUTXO(mw::db::IDBBatch& batch, const UTXO::CPtr& pUTXO);
    void SpendUTXO(mw::db::IDBBatch& batch, const Commitment& commitment);

    std::shared_ptr<mw::db::IDBWrapper> m_pDatabase;

    mmr::LeafSet::Ptr m_pLeafSet;
    mmr::MMR::Ptr m_pKernelMMR;
    mmr::MMR::Ptr m_pOutputPMMR;
    mmr::MMR::Ptr m_pRangeProofPMMR;
};

// TODO: CoinsViewMempool

END_NAMESPACE