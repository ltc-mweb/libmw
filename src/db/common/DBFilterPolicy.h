#pragma once

#include <leveldb/filter_policy.h>

class DBFilterPolicy : public leveldb::FilterPolicy
{
public:
    DBFilterPolicy() : m_pFilterPolicy(leveldb::NewBloomFilterPolicy(10)) { }
    virtual ~DBFilterPolicy() { delete m_pFilterPolicy; }

    const char* Name() const final { return "IgnoreTableFilter"; }

    void CreateFilter(const leveldb::Slice* keys, const int n, std::string* dst) const final
    {
        std::vector<leveldb::Slice> trimmed(n);
        for (int i = 0; i < n; i++)
        {
            assert(keys[i].size() > 1);

            trimmed[i] = leveldb::Slice(keys[i].data() + 1, keys[i].size() - 1);
        }

        m_pFilterPolicy->CreateFilter(trimmed.data(), n, dst);
    }

    bool KeyMayMatch(const leveldb::Slice& key, const leveldb::Slice& filter) const final
    {
        assert(key.size() > 1);

        return m_pFilterPolicy->KeyMayMatch(leveldb::Slice(key.data() + 1, key.size() - 1), filter);
    }

private:
    const leveldb::FilterPolicy* m_pFilterPolicy;
};