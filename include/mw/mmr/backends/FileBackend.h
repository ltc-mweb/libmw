#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/common/Macros.h>
#include <mw/mmr/Backend.h>
#include <mw/file/FilePath.h>
#include <mw/file/AppendOnlyFile.h>
#include <libmw/interfaces/db_interface.h>

MMR_NAMESPACE

class FileBackend : public IBackend
{
public:
    static std::shared_ptr<FileBackend> Open(
        const char dbPrefix,
        const FilePath& mmr_dir,
        const uint32_t file_index,
        const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper
    );

    FileBackend(
        const char dbPrefix,
        const FilePath& mmr_dir,
        const AppendOnlyFile::Ptr& pHashFile,
        const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper
    );

    void AddLeaf(const Leaf& leaf) final;
    void AddHash(const mw::Hash& hash) final;
    void Rewind(const LeafIndex& nextLeafIndex) final;

    uint64_t GetNumLeaves() const noexcept final;
    mw::Hash GetHash(const Index& idx) const final;
    Leaf GetLeaf(const LeafIndex& idx) const final;

    void Commit(const uint32_t file_index, const std::unique_ptr<libmw::IDBBatch>& pBatch) final;

private:
    static FilePath GetPath(const FilePath& dir, const char prefix, const uint32_t file_index);

    char m_dbPrefix;
    FilePath m_dir;
    AppendOnlyFile::Ptr m_pHashFile;
    std::vector<Leaf> m_leaves;
    std::map<mw::Hash, size_t> m_leafMap; // TODO: Can probably just be map<LeafIndex, size_t>
    std::shared_ptr<libmw::IDBWrapper> m_pDatabase;
};

END_NAMESPACE