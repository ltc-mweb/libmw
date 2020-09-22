#pragma once

#include <mw/common/Macros.h>
#include <mw/file/File.h>
#include <mw/file/MemMap.h>
#include <mw/models/crypto/Hash.h>
#include <mw/mmr/LeafIndex.h>
#include <mw/traits/Batchable.h>
#include <unordered_map>

class ILeafSetBackend;

MMR_NAMESPACE

class ILeafSet
{
public:
	using Ptr = std::shared_ptr<ILeafSet>;

	virtual ~ILeafSet() = default;

	virtual void Add(const LeafIndex& idx) = 0;
	virtual void Remove(const LeafIndex& idx) = 0;
	virtual bool Contains(const LeafIndex& idx) const noexcept = 0;
	virtual mw::Hash Root(const uint64_t numLeaves) const = 0;
	virtual uint64_t GetSize() const = 0;
	virtual uint8_t GetByte(const uint64_t byteIdx) const = 0;

	virtual void Rewind(const uint64_t numLeaves, const std::vector<LeafIndex>& leavesToAdd) = 0;
	//virtual void Snapshot(const File& snapshotFile) const = 0;
};

class LeafSet : public ILeafSet
{
public:
	using Ptr = std::shared_ptr<LeafSet>;

	LeafSet(MemMap&& mmap)
		: m_mmap(std::move(mmap)) { }

    void Add(const LeafIndex& idx) final;
    void Remove(const LeafIndex& idx) final;
    bool Contains(const LeafIndex& idx) const noexcept final;
    mw::Hash Root(const uint64_t numLeaves) const final;
    uint64_t GetSize() const;

    void Rewind(const uint64_t numLeaves, const std::vector<LeafIndex>& leavesToAdd) final;

	uint8_t GetByte(const uint64_t byteIdx) const final;

private:
	uint8_t BitToByte(const uint8_t bit) const;

	MemMap m_mmap;
	std::unordered_map<uint64_t, uint8_t> m_modifiedBytes;
};

class BackedLeafSet : public ILeafSet
{
public:
	BackedLeafSet(const ILeafSet::Ptr& pBacked)
		: m_pBacked(pBacked) { }

	void Add(const LeafIndex& idx) final;
	void Remove(const LeafIndex& idx) final;
	bool Contains(const LeafIndex& idx) const noexcept final;
	mw::Hash Root(const uint64_t numLeaves) const final;
	uint64_t GetSize() const final;

	void Rewind(const uint64_t numLeaves, const std::vector<LeafIndex>& leavesToAdd) final;

	uint8_t GetByte(const uint64_t byteIdx) const final;

private:
	uint8_t BitToByte(const uint8_t bit) const;

	ILeafSet::Ptr m_pBacked;
	std::unordered_map<uint64_t, uint8_t> m_modifiedBytes;
};

END_NAMESPACE