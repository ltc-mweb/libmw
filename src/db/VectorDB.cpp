#include <mw/db/VectorDB.h>
#include "common/Database.h"
#include "common/Serializable.h"

VectorDB::VectorDB(const std::string& name, libmw::IDBWrapper* pDBWrapper, libmw::IDBBatch* pBatch)
    : m_name("V" + name), m_pDatabase(std::make_unique<Database>(pDBWrapper, pBatch))
{
}

VectorDB::~VectorDB() {}

uint64_t VectorDB::Size() const
{
    auto pSize = m_pDatabase->Get<SerializableInt>(m_name, "Size");
    if (pSize == nullptr) {
        return 0;
    }
    return pSize->item->Get();
}

std::string VectorDB::IndexToString(uint64_t index)
{
    Serializer serializer;
    serializer.Append(index);
    return std::string(serializer.vec().begin(), serializer.vec().end());
}

std::unique_ptr<std::vector<uint8_t>> VectorDB::Get(uint64_t index) const
{
    if (index >= Size()) {
        return nullptr;
    }
    auto pItem = m_pDatabase->Get<SerializableVec>(m_name, IndexToString(index));
    if (pItem == nullptr) {
        return nullptr;
    }
    return std::make_unique<std::vector<uint8_t>>(std::move(pItem->item->Get()));
}

void VectorDB::Add(std::vector<std::vector<uint8_t>>&& items)
{
    if (items.empty()) {
        return;
    }
    uint64_t size = Size();
    std::vector<DBEntry<SerializableVec>> entries;
    std::transform(
        items.begin(), items.end(),
        std::back_inserter(entries),
        [&size](std::vector<uint8_t>& item) {
            return DBEntry<SerializableVec>(IndexToString(size++), std::move(item));
        }
    );
    items.clear();

    DBEntry<SerializableInt> entry("Size", size);
    m_pDatabase->Put(m_name, std::vector<DBEntry<SerializableInt>>{entry});
    m_pDatabase->Put(m_name, entries);
}

void VectorDB::Rewind(uint64_t nextIndex)
{
    DBEntry<SerializableInt> entry("Size", nextIndex);
    m_pDatabase->Put(m_name, std::vector<DBEntry<SerializableInt>>{entry});
}

void VectorDB::RemoveAt(const std::vector<uint64_t>& indexes)
{
    for (uint64_t index : indexes) {
        m_pDatabase->Delete(m_name, IndexToString(index));
    }
}

void VectorDB::RemoveAll()
{
    m_pDatabase->DeleteAll(m_name);
}