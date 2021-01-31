#pragma once

// Copyright (c) 2018-2020 David Burkett
// Copyright (c) 2020 The Litecoin Developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <libmw/defs.h>

LIBMW_NAMESPACE

class IChainIterator
{
public:
    virtual ~IChainIterator() = default;

    virtual void Next() noexcept = 0;
    virtual bool Valid() const noexcept = 0;

    virtual uint64_t GetHeight() const = 0;
    virtual libmw::BlockHash GetCanonicalHash() const = 0;

    virtual libmw::HeaderRef GetHeader() const = 0;
    virtual libmw::HeaderAndPegsRef GetHeaderAndPegs() const = 0;
    virtual libmw::BlockRef GetBlock() const = 0;
};

//
// Interface for accessing blocks in the chain.
// This must be implemented by the libmw consumer.
//
class IChain
{
public:
    using Ptr = std::shared_ptr<libmw::IChain>;

    virtual ~IChain() = default;

    virtual std::unique_ptr<IChainIterator> NewIterator() = 0;
};

END_NAMESPACE // libmw