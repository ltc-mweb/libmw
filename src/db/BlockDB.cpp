#include "BlockDB.h"

#include <mw/common/Logger.h>

static const DBTable HEADER_TABLE = { 'H' };
static const DBTable BLOCK_TABLE = { 'B' };
static const DBTable UTXO_TABLE = { 'U', DBTable::Options({ true /* allowDuplicates */ }) };

Locked<IBlockDB> BlockDBFactory::Open(const FilePath& chainPath)
{
    auto pDatabase = Database::Open(chainPath.GetChild("blocks"));
    auto pChainStore = ChainStore::Load(chainPath.GetChild("chain"));

    return Locked<IBlockDB>(std::make_shared<BlockDB>(pDatabase, pChainStore));
}

mw::Header::CPtr BlockDB::GetHeaderByHash(const mw::Hash& hash) const noexcept
{
    LOG_TRACE_F("Loading header {}", hash);

    auto pEntry = m_pDatabase->Get<mw::Header>(HEADER_TABLE, hash.ToHex());
    if (pEntry != nullptr)
    {
        LOG_DEBUG_F("Header found for hash {}", hash);
        return pEntry->item;
    }
    else
    {
        LOG_DEBUG_F("No header found for hash {}", hash);
        return nullptr;
    }
}

std::vector<mw::Header::CPtr> BlockDB::GetHeadersByHash(const std::vector<mw::Hash>& hashes) const noexcept
{
    LOG_TRACE_F("Loading {} headers", hashes.size());

    std::vector<mw::Header::CPtr> headers;
    headers.reserve(hashes.size());

    for (const mw::Hash& hash : hashes)
    {
        auto pEntry = m_pDatabase->Get<mw::Header>(HEADER_TABLE, hash.ToHex());
        if (pEntry != nullptr)
        {
            LOG_TRACE_F("Header found for hash {}", hash);
            headers.emplace_back(pEntry->item);
        }
        else
        {
            LOG_DEBUG_F("No header found for hash {}", hash);
        }
    }

    LOG_DEBUG_F("Found {}/{} headers", headers.size(), hashes.size());
    return headers;
}

void BlockDB::AddHeader(const mw::Header::CPtr& pHeader)
{
    LOG_TRACE_F("Adding header {}", pHeader);

    std::vector<DBEntry<mw::Header>> entries({ BlockDB::ToHeaderEntry(pHeader) });
    m_pDatabase->Put(HEADER_TABLE, entries);
}

void BlockDB::AddHeaders(const std::vector<mw::Header::CPtr>& headers)
{
    LOG_TRACE_F("Adding {} headers", headers.size());

    std::vector<DBEntry<mw::Header>> entries;
    entries.reserve(headers.size());

    std::transform(
        headers.cbegin(), headers.cend(),
        std::back_inserter(entries),
        BlockDB::ToHeaderEntry
    );
    m_pDatabase->Put(HEADER_TABLE, entries);
}

mw::Block::CPtr BlockDB::GetBlockByHash(const mw::Hash& hash) const noexcept
{
    LOG_TRACE_F("Loading block {}", hash);

    auto pEntry = m_pDatabase->Get<mw::Block>(BLOCK_TABLE, hash.ToHex());
    if (pEntry != nullptr)
    {
        LOG_DEBUG_F("IBlock found for hash {}", hash);
        return pEntry->item;
    }
    else
    {
        LOG_DEBUG_F("No block found for hash {}", hash);
        return nullptr;
    }
}

mw::Block::CPtr BlockDB::GetBlockByHeight(const uint64_t height) const noexcept
{
    LOG_TRACE_F("Loading block {}", height);

    boost::optional<mw::Hash> hashOpt = m_pChainStore->GetHashByHeight(height);
    if (hashOpt.has_value())
    {
        LOG_DEBUG_F("Hash {} found for height {}", hashOpt.value(), height);
        return GetBlockByHash(hashOpt.value());
    }
    else
    {
        LOG_DEBUG_F("No hash found at height {}", height);
        return nullptr;
    }
}

void BlockDB::AddBlock(const mw::Block::CPtr& pBlock)
{
    LOG_TRACE_F("Saving block {}", pBlock);

    std::vector<DBEntry<mw::Block>> entries({ BlockDB::ToBlockEntry(pBlock) });
    m_pDatabase->Put<mw::Block>(BLOCK_TABLE, entries);
}

void BlockDB::RemoveOldBlocks(const uint64_t height)
{
    LOG_TRACE_F("Removing blocks before height {}", height);

    // TODO: Implement
}

std::unordered_map<Commitment, UTXO::CPtr> BlockDB::GetUTXOs(const std::vector<Commitment>& commitments) const noexcept
{
    std::unordered_map<Commitment, UTXO::CPtr> utxos;

    for (const Commitment& commitment : commitments)
    {
        auto pUTXO = m_pDatabase->Get<UTXO>(UTXO_TABLE, commitment.ToHex());
        if (pUTXO != nullptr)
        {
            utxos.insert({ commitment, pUTXO->item });
        }
    }

    return utxos;
}

void BlockDB::AddUTXOs(const std::vector<UTXO::CPtr>& utxos)
{
    // TODO: Implement
}

void BlockDB::RemoveUTXOs(const std::vector<Commitment>& commitment)
{
    // TODO: Implement
}

void BlockDB::RemoveAllUTXOs()
{
    // TODO: Implement
}