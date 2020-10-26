#include <mw/db/LeafDB.h>
#include "common/Database.h"
#include "common/SerializableVec.h"

LeafDB::LeafDB(const char prefix, libmw::IDBWrapper* pDBWrapper, libmw::IDBBatch* pBatch)
    : m_prefix(prefix), m_pDatabase(std::make_unique<Database>(pDBWrapper, pBatch))
{
}

LeafDB::~LeafDB() {}

std::unique_ptr<mmr::Leaf> LeafDB::Get(const mmr::LeafIndex& idx, mw::Hash&& hash) const
{
    auto pVec = m_pDatabase->Get<SerializableVec>(m_prefix, hash.ToHex());
    if (pVec == nullptr) {
        return nullptr;
    }
    return std::make_unique<mmr::Leaf>(idx, std::move(hash), std::vector<uint8_t>(pVec->item->Get()));
}

void LeafDB::Add(const std::vector<mmr::Leaf>& leaves)
{
    if (leaves.empty()) {
        return;
    }
    std::vector<DBEntry<SerializableVec>> entries;
    std::transform(
        leaves.cbegin(), leaves.cend(),
        std::back_inserter(entries),
        [](const mmr::Leaf& leaf) {
            return DBEntry<SerializableVec>(leaf.GetHash().ToHex(), std::vector<uint8_t>(leaf.vec()));
        }
    );
    m_pDatabase->Put(m_prefix, entries);
}

void LeafDB::Remove(const std::vector<mw::Hash>& hashes)
{
    for (const mw::Hash& hash : hashes) {
        m_pDatabase->Delete(m_prefix, hash.ToHex());
    }
}

void LeafDB::RemoveAll()
{
    m_pDatabase->DeleteAll(m_prefix);
}