#pragma once

#include <mw/file/File.h>
#include <mw/file/FilePath.h>
#include <mw/file/MemMap.h>
#include <mw/common/Logger.h>
#include <mw/traits/Batchable.h>
#include <libmw/interfaces.h>

class AppendOnlyFile : public Traits::IBatchable
{
public:
    using Ptr = std::shared_ptr<AppendOnlyFile>;

    AppendOnlyFile(const File& file, const size_t fileSize) noexcept
        : m_file(file),
        m_mmap(file.GetPath()),
        m_fileSize(fileSize),
        m_bufferIndex(fileSize)
    {

    }
    virtual ~AppendOnlyFile() = default;

    static AppendOnlyFile::Ptr Load(const FilePath& path)
    {
        File file(path);
        file.Create();

        auto pAppendOnlyFile = std::make_shared<AppendOnlyFile>(file, file.GetSize());
        pAppendOnlyFile->m_mmap.Map();
        return pAppendOnlyFile;
    }

    void Commit(const std::unique_ptr<libmw::IDBBatch>& pBatch = nullptr) final
    {
        if (m_fileSize == m_bufferIndex && m_buffer.empty())
        {
            return;
        }

        if (m_fileSize < m_bufferIndex)
        {
            ThrowFile_F("Buffer index is past the end of {}", m_file);
        }

        m_mmap.Unmap();

        m_file.Write(m_bufferIndex, m_buffer, true);
        m_fileSize = m_file.GetSize();
        m_bufferIndex = m_fileSize;
        m_buffer.clear();

        m_mmap.Map();
    }

    void Rollback() noexcept final
    {
        m_bufferIndex = m_fileSize;
        m_buffer.clear();
    }

    void Append(const std::vector<uint8_t>& data)
    {
        m_buffer.insert(m_buffer.end(), data.cbegin(), data.cend());
    }

    void Rewind(const uint64_t nextPosition)
    {
        assert(m_fileSize == m_bufferIndex);

        if (nextPosition > (m_bufferIndex + m_buffer.size()))
        {
            ThrowFile_F("Tried to rewind past end of {}", m_file);
        }

        if (nextPosition <= m_bufferIndex)
        {
            m_buffer.clear();
            m_bufferIndex = nextPosition;
        }
        else
        {
            m_buffer.erase(m_buffer.begin() + nextPosition - m_bufferIndex, m_buffer.end());
        }
    }

    uint64_t GetSize() const noexcept
    {
        return m_bufferIndex + m_buffer.size();
    }

    std::vector<uint8_t> Read(const uint64_t position, const uint64_t numBytes) const
    {
        if ((position + numBytes) > (m_bufferIndex + m_buffer.size()))
        {
            ThrowFile_F("Tried to read past end of {}", m_file);
        }

        if (position < m_bufferIndex)
        {
            // TODO: Read from mapped and then from buffer, if necessary
            return m_mmap.Read(position, numBytes);
        }
        else
        {
            auto begin = m_buffer.cbegin() + position - m_bufferIndex;
            auto end = begin + numBytes;
            return std::vector<uint8_t>(begin, end);
        }
    }

private:
    File m_file;
    MemMap m_mmap;
    uint64_t m_fileSize;

    uint64_t m_bufferIndex;
    std::vector<uint8_t> m_buffer;
};