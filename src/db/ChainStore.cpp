#include "ChainStore.h"

ChainStore::Ptr ChainStore::Load(const FilePath& chainPath)
{
    // TODO: Implement
    return std::make_shared<ChainStore>();
}

boost::optional<mw::Hash> ChainStore::GetHashByHeight(const uint64_t height) const noexcept
{
    // TODO: Implement
    return boost::none;
}