#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/common/Macros.h>
#include <mw/mmr/Backend.h>
#include <mw/mmr/Node.h>
#include <mw/file/FilePath.h>
#include <mw/file/AppendOnlyFile.h>
#include <boost/optional.hpp>
#include <cassert>

MMR_NAMESPACE

// TODO: Add pruning support
// TODO: Just use pmmr_hash, and rely on database for storing the data
class FileBackend : public IBackend
{
    struct PosEntry
    {
        static const uint8_t LENGTH{ 10 };

        uint64_t position;
        uint16_t size;
    };

public:
    static std::shared_ptr<FileBackend> Open(const FilePath& path, const boost::optional<uint16_t>& fixedLengthOpt)
    {
        auto pBackend = std::make_shared<FileBackend>(
            AppendOnlyFile::Load(path.GetChild("pmmr_hash.bin")),
            AppendOnlyFile::Load(path.GetChild("pmmr_data.bin")),
            fixedLengthOpt.value_or(0)
        );

        if (!fixedLengthOpt.has_value())
        {
            pBackend->m_pPositionFile = AppendOnlyFile::Load(path.GetChild("pmmr_pos.bin"));
        }

        return pBackend;
    }

    FileBackend(const AppendOnlyFile::Ptr& pHashFile, const AppendOnlyFile::Ptr& pDataFile, const uint16_t fixedLength)
        : m_pHashFile(pHashFile), m_pDataFile(pDataFile), m_fixedLength(fixedLength) { }

    void AddLeaf(const Leaf& leaf) final
    {
        AppendData(leaf.vec());
        AddHash(leaf.GetHash());

        auto rightHash = leaf.GetHash();
        auto nextIdx = leaf.GetNodeIndex().GetNext();
        while (!nextIdx.IsLeaf())
        {
            const mw::Hash leftHash = GetHash(nextIdx.GetLeftChild());
            const Node node = Node::CreateParent(nextIdx, leftHash, rightHash);

            AddHash(node.GetHash());
            rightHash = node.GetHash();
            nextIdx = nextIdx.GetNext();
        }
    }

    void AddHash(const mw::Hash& hash) final { m_pHashFile->Append(hash.vec()); }

    void Rewind(const LeafIndex& nextLeafIndex) final
    {
        if (nextLeafIndex.GetLeafIndex() == 0) {
            if (m_pPositionFile != nullptr) {
                m_pPositionFile->Rewind(0);
            }

            m_pDataFile->Rewind(0);
            m_pHashFile->Rewind(0);
            return;
        }

        if (m_pPositionFile != nullptr) {
            const PosEntry posEntry = GetPosEntry(nextLeafIndex.GetLeafIndex() - 1);
            m_pPositionFile->Rewind(nextLeafIndex.GetLeafIndex() * PosEntry::LENGTH);
            m_pDataFile->Rewind(posEntry.position + posEntry.size);
        } else {
            m_pDataFile->Rewind(nextLeafIndex.GetLeafIndex() * m_fixedLength);
        }

        m_pHashFile->Rewind(nextLeafIndex.GetPosition() * 32);
    }

    uint64_t GetNumLeaves() const noexcept final
    {
        if (m_pPositionFile != nullptr)
        {
            return m_pPositionFile->GetSize() / PosEntry::LENGTH;
        }
        else
        {
            return m_pDataFile->GetSize() / m_fixedLength;
        }
    }

    mw::Hash GetHash(const Index& idx) const final
    {
        return mw::Hash(m_pHashFile->Read(idx.GetPosition() * mw::Hash::size(), mw::Hash::size()));
    }

    Leaf GetLeaf(const LeafIndex& idx) const final
    {
        const uint64_t leafIndex = idx.GetLeafIndex();
        if (m_pPositionFile != nullptr)
        {
            PosEntry posEntry = GetPosEntry(leafIndex);
            std::vector<uint8_t> data = m_pDataFile->Read(posEntry.position, posEntry.size);
            return Leaf::Create(idx, std::move(data));
        }
        else
        {
            std::vector<uint8_t> data = m_pDataFile->Read(leafIndex * m_fixedLength, m_fixedLength);
            return Leaf::Create(idx, std::move(data));
        }
    }

    void Commit() final
    {
        m_pHashFile->Commit();
        m_pDataFile->Commit();

        if (m_pPositionFile != nullptr)
        {
            m_pPositionFile->Commit();
        }
    }

    void Rollback() noexcept final
    {
        m_pHashFile->Rollback();
        m_pDataFile->Rollback();

        if (m_pPositionFile != nullptr)
        {
            m_pPositionFile->Rollback();
        }
    }

private:
    PosEntry GetPosEntry(const uint64_t leafIndex) const
    {
        assert(m_pPositionFile != nullptr);

        std::vector<uint8_t> data = m_pPositionFile->Read(leafIndex * PosEntry::LENGTH, PosEntry::LENGTH);

        Deserializer deserializer(std::move(data));
        const uint64_t position = deserializer.Read<uint64_t>();
        const uint16_t size = deserializer.Read<uint16_t>();
        return PosEntry{ position, size };
    }

    void AppendData(const std::vector<unsigned char>& data)
    {
        if (m_pPositionFile != nullptr)
        {
            const auto serialized = Serializer()
                .Append<uint64_t>(m_pDataFile->GetSize())
                .Append<uint16_t>((uint16_t)data.size())
                .vec();
            m_pPositionFile->Append(serialized);
        }

        m_pDataFile->Append(data);
    }

    AppendOnlyFile::Ptr m_pHashFile;
    AppendOnlyFile::Ptr m_pDataFile;
    AppendOnlyFile::Ptr m_pPositionFile;

    uint16_t m_fixedLength;
};

END_NAMESPACE