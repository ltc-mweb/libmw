#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <mw/models/crypto/Commitment.h>

namespace Traits
{
    class ICommitted
    {
    public:
        virtual ~ICommitted() = default;

        virtual const Commitment& GetCommitment() const noexcept = 0;
    };
}