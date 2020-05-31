#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/common/Macros.h>
#include <mw/mmr/Index.h>
#include <mw/models/crypto/Hash.h>
#include <mw/crypto/Crypto.h>

MMR_NAMESPACE

class Node
{
public:
    static Node CreateParent(const Index& index, const Hash& leftHash, const Hash& rightHash)
    {
        Serializer hashSerializer;
        hashSerializer.Append<uint64_t>(index.GetPosition());
        hashSerializer.Append(leftHash);
        hashSerializer.Append(rightHash);
        Hash hash = Crypto::Blake2b(hashSerializer.vec());

        return Node(index, std::move(hash));
    }

    const Index& GetIndex() const noexcept { return m_index; }
    const Hash& GetHash() const noexcept { return m_hash; }
    uint64_t GetHeight() const noexcept { return m_index.GetHeight(); }

private:
    Node(const Index& index, Hash&& hash)
        : m_index(index), m_hash(std::move(hash)) { }

    Index m_index;
    Hash m_hash;
};

END_NAMESPACE