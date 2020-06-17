#pragma once

#include "DBTable.h"
#include "DBEntry.h"
#include "OrderedMultimap.h"

#include <mw/exceptions/DatabaseException.h>
#include <mw/serialization/Serializer.h>
#include <mw/traits/Serializable.h>
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

class DBTransaction
{
public:
    using Ptr = std::shared_ptr<DBTransaction>;

    DBTransaction(leveldb::DB* pDB) : m_pDB(pDB) { }

    template<typename T,
        typename SFINAE = typename std::enable_if_t<std::is_base_of_v<Traits::ISerializable, T>>>
    DBTransaction& Put(const DBTable& table, const std::vector<DBEntry<T>>& entries)
    {
        for (const auto& entry : entries)
        {
            const std::string key = table.BuildKey(entry);

            Serializer serializer;
            serializer.Append(entry.item);

            m_batch.Put(leveldb::Slice(key), leveldb::Slice((const char*)serializer.data(), serializer.size()));
            m_added.insert({ key, entry.item });
        }

        return *this;
    }

    template<typename T,
        typename SFINAE = typename std::enable_if_t<std::is_base_of_v<Traits::ISerializable, T>>>
    std::unique_ptr<DBEntry<T>> Get(const DBTable& table, const std::string& key) const noexcept
    {
        auto iter = m_added.find_last(table.BuildKey(key));
        if (iter != nullptr)
        {
            auto pObject = std::dynamic_pointer_cast<const T>(iter);
            if (pObject != nullptr)
            {
                return std::make_unique<DBEntry<T>>(key, pObject);
            }
        }

        std::string itemStr;
        leveldb::Status status = m_pDB->Get(leveldb::ReadOptions(), table.BuildKey(key), &itemStr);
        if (status.ok())
        {
            Deserializer deserializer(std::vector<uint8_t>(itemStr.cbegin(), itemStr.cend()));
            return std::make_unique<DBEntry<T>>(key, T::Deserialize(deserializer));
        }

        return nullptr;
    }

    void Delete(const DBTable& table, const std::string& key)
    {
        m_batch.Delete(leveldb::Slice(table.BuildKey(key)));
        m_added.erase(table.BuildKey(key));
    }

    void Commit()
    {
        leveldb::Status status = m_pDB->Write(leveldb::WriteOptions(), &m_batch);
        if (!status.ok())
        {
            ThrowDatabase_F("Commit failed with status {}", status.ToString());
        }
    }

private:
    leveldb::DB* m_pDB;
    leveldb::WriteBatch m_batch;
    OrderedMultimap<std::string, Traits::ISerializable> m_added;
    //std::unordered_map<std::string, std::shared_ptr<const Traits::ISerializable>> m_added;
};