#pragma once

#include <mw/file/FilePath.h>
#include <mw/traits/Printable.h>
#include <map>

class File : public Traits::IPrintable
{
public:
    File(const FilePath& path) : m_path(path) { }
    File(FilePath&& path) : m_path(std::move(path)) { }
    virtual ~File() = default;

    // Creates an empty file if it doesn't already exist
    void Create();

    void Truncate(const uint64_t size);
    void Rename(const std::string& filename);
    std::vector<uint8_t> ReadBytes() const;
    void Write(const std::vector<uint8_t>& bytes);
    void Write(
        const size_t startIndex,
        const std::vector<uint8_t>& bytes,
        const bool truncate
    );
    void WriteBytes(const std::map<uint64_t, uint8_t>& bytes);
    size_t GetSize() const;

    const FilePath& GetPath() const noexcept { return m_path; }
    const fs::path& GetFSPath() const noexcept { return m_path.ToPath(); }

    //
    // Traits
    //
    std::string Format() const final { return m_path.ToPath().u8string(); }

private:
    FilePath m_path;
};