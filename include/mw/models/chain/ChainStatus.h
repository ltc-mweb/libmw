#pragma once

#include <mw/models/crypto/Hash.h>
#include <memory>
#include <shared_mutex>

enum class ETxHashSetStatus
{
	NEEDED,
	INVALID,
	PROCESSING,
	VALID
};

struct Tip
{
	uint64_t m_height;
	Hash m_hash;
};

class ChainStatus
{
public:
	using Ptr = std::shared_ptr<ChainStatus>;
	using CPtr = std::shared_ptr<const ChainStatus>;

	ChainStatus()
		: m_txHashSetStatus(ETxHashSetStatus::NEEDED),
		m_headerTip({ 0, ZERO_HASH }),
		m_blockTip({ 0, ZERO_HASH })
	{

	}

	ETxHashSetStatus GetTxHashSetStatus() const noexcept
	{
		std::shared_lock<std::shared_mutex> readLock(m_mutex);

		return m_txHashSetStatus;
	}

	Tip GetHeaderTip() const noexcept
	{
		std::shared_lock<std::shared_mutex> readLock(m_mutex);

		return m_headerTip;
	}

	uint64_t GetHeaderHeight() const noexcept
	{
		std::shared_lock<std::shared_mutex> readLock(m_mutex);

		return m_headerTip.m_height;
	}

	Tip GetBlockTip() const noexcept
	{
		std::shared_lock<std::shared_mutex> readLock(m_mutex);

		return m_blockTip;
	}

	uint64_t GetBlockHeight() const noexcept
	{
		std::shared_lock<std::shared_mutex> readLock(m_mutex);

		return m_blockTip.m_height;
	}

	void UpdateTxHashSetStatus(const ETxHashSetStatus txHashSetStatus) noexcept
	{
		std::unique_lock<std::shared_mutex> writeLock(m_mutex);

		m_txHashSetStatus = txHashSetStatus;
	}

	void UpdateHeaderTip(const uint64_t height, const Hash& hash) noexcept
	{
		std::unique_lock<std::shared_mutex> writeLock(m_mutex);

		m_headerTip = { height, hash };
	}

	void UpdateBlockTip(const uint64_t height, const Hash& hash) noexcept
	{
		std::unique_lock<std::shared_mutex> writeLock(m_mutex);

		m_blockTip = { height, hash };
	}

private:
	mutable std::shared_mutex m_mutex;

	ETxHashSetStatus m_txHashSetStatus;
	Tip m_headerTip;
	Tip m_blockTip;
};