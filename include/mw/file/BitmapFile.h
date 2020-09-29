#pragma once

#include <mw/file/MemMap.h>
#include <mw/traits/Batchable.h>
#include <mw/util/BitUtil.h>
#include <fstream>
#include <functional>
#include <algorithm>
#include <map>
#include <memory>
#include <cassert>

// NOTE: Uses bit positions numbered from 0-7, starting at the left.
// For example, 65 (01000001) has bit positions 1 and 7 set.
class BitmapFile : public Traits::IBatchable
{
public:
    virtual ~BitmapFile() = default;

    static std::shared_ptr<BitmapFile> Load(const File& file)
    {
        auto pBitmapFile = std::shared_ptr<BitmapFile>(new BitmapFile(file));
        pBitmapFile->Load();
        return pBitmapFile;
    }

    void Commit() final
    {
        if (!m_modifiedBytes.empty())
        {
            m_memmap.Unmap();
            m_file.WriteBytes(m_modifiedBytes);
            m_modifiedBytes.clear();
            m_memmap.Map();
            SetDirty(false);
        }
    }

    void Rollback() noexcept final
    {
        m_modifiedBytes.clear();
        SetDirty(false);
    }

    bool IsSet(const uint64_t position) const
    {
        return GetByte(position / 8) & BitToByte(position % 8);
    }

    void Set(const uint64_t position)
    {
        SetDirty(true);
        uint8_t byte = GetByte(position / 8);
        byte |= BitToByte(position % 8);
        m_modifiedBytes[position / 8] = byte;
    }

    //void Set(const Roaring& positionsToSet)
    //{
    //    for (auto iter = positionsToSet.begin(); iter != positionsToSet.end(); iter++)
    //    {
    //        Set(iter.i.current_value - 1);
    //    }
    //}

    void Unset(const uint64_t position)
    {
        SetDirty(true);
        uint8_t byte = GetByte(position / 8);
        byte &= (0xff ^ BitToByte(position % 8));
        m_modifiedBytes[position / 8] = byte;
    }

    //void Unset(const Roaring& positionsToUnset)
    //{
    //    for (auto iter = positionsToUnset.begin(); iter != positionsToUnset.end(); iter++)
    //    {
    //        Unset(iter.i.current_value - 1);
    //    }
    //}

    const bool& operator[] (const size_t position) const
    {
        return IsSet(position) ? s_true : s_false;
    }

    //void Rewind(const size_t size, const Roaring& positionsToAdd)
    //{
    //    Set(positionsToAdd);

    //    size_t currentSize = GetNumBytes() * 8;
    //    for (size_t i = size; i < currentSize; i++)
    //    {
    //        Unset(i);
    //    }
    //}

private:
    BitmapFile(const File& file) : m_file(file), m_memmap(file) { }

    void Load()
    {
        m_file.Create();
        m_memmap.Map();
    }

    uint8_t GetByte(const uint64_t byteIndex) const
    {
        auto iter = m_modifiedBytes.find(byteIndex);
        if (iter != m_modifiedBytes.cend())
        {
            return iter->second;
        }
        else if (byteIndex < m_memmap.size())
        {
            return m_memmap.ReadByte(byteIndex);
        }

        return 0;
    }

    uint64_t GetNumBytes() const
    {
        size_t size = 0;
        if (!m_modifiedBytes.empty())
        {
            size = m_modifiedBytes.crbegin()->first + 1;
        }

        if (!m_memmap.empty())
        {
            size = (std::max)(size, m_memmap.size() + 1); // TODO: Why +1?
        }

        return size;
    }

    // Returns a byte with the given bit (0-7) set.
    // Example: BitToByte(2) returns 32 (00100000).
    uint8_t BitToByte(const uint8_t bit) const noexcept
    {
        assert(bit <= 7);
        return 1 << (7 - bit);
    }

    bool IsBitSet(const uint8_t byte, const uint64_t position) const noexcept
    {
        const uint8_t bitPosition = (uint8_t)position % 8;

        return (byte >> (7 - bitPosition)) & 1;
    }

    File m_file;
    std::unordered_map<uint64_t, uint8_t> m_modifiedBytes;
    MemMap m_memmap;

    static const bool s_true{ false };
    static const bool s_false{ false };
};