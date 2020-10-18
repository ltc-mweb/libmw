#pragma once

#include <libmw/interfaces.h>

// Forward Declarations
class Database;

class VectorDB
{
public:
    VectorDB(const std::string& name, libmw::IDBWrapper* pDBWrapper, libmw::IDBBatch* pBatch = nullptr);
    ~VectorDB();

    uint64_t Size() const;
    std::unique_ptr<std::vector<uint8_t>> Get(uint64_t index) const;
    void Add(std::vector<std::vector<uint8_t>>&& items);
    void Rewind(uint64_t nextIndex);
    void RemoveAt(const std::vector<uint64_t>& indexes);
    void RemoveAll();

private:
    static std::string IndexToString(uint64_t index);

    std::string m_name;
    std::unique_ptr<Database> m_pDatabase;
};