#include <mw/mmr/backends/FileBackend.h>
#include <mw/mmr/Node.h>
#include <mw/db/LeafDB.h>
#include <mw/exceptions/NotFoundException.h>

// TODO: Add pruning support

std::shared_ptr<mmr::FileBackend> mmr::FileBackend::Open(
    const char dbPrefix,
    const FilePath& mmr_dir,
    const uint32_t file_index,
    const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper)
{
    const FilePath path = GetPath(mmr_dir, dbPrefix, file_index);
    return std::make_shared<FileBackend>(
        dbPrefix,
        mmr_dir,
        AppendOnlyFile::Load(path),
        pDBWrapper
    );
}

mmr::FileBackend::FileBackend(
    const char dbPrefix,
    const FilePath& mmr_dir,
    const AppendOnlyFile::Ptr& pHashFile,
    const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper)
    : m_dbPrefix(dbPrefix), m_dir(mmr_dir), m_pHashFile(pHashFile), m_pDatabase(pDBWrapper)
{
}

void mmr::FileBackend::AddLeaf(const Leaf& leaf)
{
    m_leafMap[leaf.GetHash()] = m_leaves.size();
    m_leaves.push_back(leaf);
    AddHash(leaf.GetHash());

    auto rightHash = leaf.GetHash();
    auto nextIdx = leaf.GetNodeIndex().GetNext();
    while (!nextIdx.IsLeaf()) {
        const mw::Hash leftHash = GetHash(nextIdx.GetLeftChild());
        const Node node = Node::CreateParent(nextIdx, leftHash, rightHash);

        AddHash(node.GetHash());
        rightHash = node.GetHash();
        nextIdx = nextIdx.GetNext();
    }
}

void mmr::FileBackend::AddHash(const mw::Hash& hash)
{
    m_pHashFile->Append(hash.vec());
}

void mmr::FileBackend::Rewind(const LeafIndex& nextLeafIndex)
{
    m_pHashFile->Rewind(nextLeafIndex.GetPosition() * 32);
}

uint64_t mmr::FileBackend::GetNumLeaves() const noexcept
{
    return Index::At(m_pHashFile->GetSize() / mw::Hash::size()).GetLeafIndex();
}

mw::Hash mmr::FileBackend::GetHash(const Index& idx) const
{
    return mw::Hash(m_pHashFile->Read(idx.GetPosition() * mw::Hash::size(), mw::Hash::size()));
}

mmr::Leaf mmr::FileBackend::GetLeaf(const LeafIndex& idx) const
{
    mw::Hash hash = GetHash(idx.GetNodeIndex());
    auto it = m_leafMap.find(hash);
    if (it != m_leafMap.end()) {
        return m_leaves[it->second];
    }

    LeafDB ldb(m_dbPrefix, m_pDatabase.get());
    auto pLeaf = ldb.Get(idx, std::move(hash));
    if (!pLeaf) {
        ThrowNotFound_F("Can't get leaf at position {} with hash {}", idx.GetPosition(), hash);
    }

    return std::move(*pLeaf);
}

void mmr::FileBackend::Commit(const uint32_t file_index, const std::unique_ptr<libmw::IDBBatch>& pBatch)
{
    m_pHashFile->Commit(GetPath(m_dir, m_dbPrefix, file_index));

    // Update database
    LeafDB(m_dbPrefix, m_pDatabase.get(), pBatch.get())
        .Add(m_leaves);

    m_leaves.clear();
    m_leafMap.clear();
}

FilePath mmr::FileBackend::GetPath(const FilePath& dir, const char prefix, const uint32_t file_index)
{
    return dir.GetChild(StringUtil::Format("{}{:0>6}.dat", prefix, file_index));;
}