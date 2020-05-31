#include "ChainStore.h"

ChainStore::Ptr ChainStore::Load(const FilePath& chainPath)
{
    // TODO: Implement
    return std::make_shared<ChainStore>();
}

tl::optional<Hash> ChainStore::GetHashByHeight(const uint64_t height) const noexcept
{
    // TODO: Implement
    return tl::nullopt;
}