#pragma once

#include "DBTable.h"
#include "DBTransaction.h"
#include "DBEntry.h"
#include "DBLogger.h"
#include "DBFilterPolicy.h"

#include <mw/file/FilePath.h>
#include <mw/traits/Batchable.h>
#include <mw/common/Lock.h>

#include <leveldb/db.h>
#include <leveldb/filter_policy.h>
#include <vector>
#include <cassert>
#include <memory>

class Database : public Traits::IBatchable
{
public:
    using Ptr = std::shared_ptr<Database>;

    static Database::Ptr Open(const FilePath& path)
    {
        path.CreateDirIfMissing();

        std::unique_ptr<DBLogger> pLogger = std::make_unique<DBLogger>();

        leveldb::Options options;
        options.create_if_missing = true;
        options.info_log = new DBLogger();
        options.filter_policy = new DBFilterPolicy();

        // Snappy compression is fast, but not useful for pseudorandom data like hashes & commitments.
        options.compression = leveldb::kNoCompression;

        leveldb::DB* pDB = nullptr;
        leveldb::Status status = leveldb::DB::Open(options, path.u8string(), &pDB); // TODO: Add tests for windows unicode paths.
        if (!status.ok())
        {
            ThrowDatabase_F("Open failed with status {}", status.ToString());
        }

        return std::shared_ptr<Database>(new Database(std::move(options), pDB));
    }

    virtual ~Database()
    {
        delete m_pDB;
        delete m_options.filter_policy;
        delete m_options.info_log;
    }

    //
    // Operations
    //
    template<typename T,
        typename SFINAE = typename std::enable_if_t<std::is_base_of_v<Traits::ISerializable, T>>>
    std::unique_ptr<DBEntry<T>> Get(const DBTable& table, const std::string& key) const noexcept
    {
        if (m_pTx != nullptr)
        {
            return m_pTx->Get<T>(table, key);
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

    template<typename T,
        typename SFINAE = typename std::enable_if_t<std::is_base_of_v<Traits::ISerializable, T>>>
    void Put(const DBTable& table, const std::vector<DBEntry<T>>& entries)
    {
        assert(!entries.empty());

        if (m_pTx != nullptr)
        {
            m_pTx->Put(table, entries);
        }
        else
        {
            DBTransaction(m_pDB).Put(table, entries).Commit();
        }
    }

    // FUTURE: std::vector<DBEntry<T>> CustomGet(const DBTable& table, const Query& query) const;

    //
    // Batchable
    //
    void OnInitWrite() noexcept final
    {
        assert(m_pTx == nullptr);
        m_pTx = std::make_shared<DBTransaction>(m_pDB);
    }

    void OnEndWrite() noexcept final
    {
        m_pTx.reset();
    }

    void Commit() final
    {
        assert(m_pTx != nullptr);
        m_pTx->Commit();
    }
    void Rollback() noexcept final { m_pTx.reset(); }
    
private:
    Database(leveldb::Options&& options, leveldb::DB* pDB) noexcept
        : m_options(std::move(options)), m_pDB(pDB), m_pTx(nullptr) { }

    leveldb::Options m_options;
    leveldb::DB* m_pDB;
    DBTransaction::Ptr m_pTx;
};