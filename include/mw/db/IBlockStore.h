#pragma once

#include <mw/common/Macros.h>
#include <mw/exceptions/NotFoundException.h>
#include <mw/models/block/Block.h>
#include <mw/models/block/HeaderAndPegs.h>

MW_NAMESPACE

//
// Interface for looking up blocks and headers.
// This must be implemented by the libmw consumer.
//
class IBlockStore
{
public:
    virtual ~IBlockStore() = default;

    virtual mw::Header::CPtr GetHeader(const uint64_t height) const /*throw(NotFoundException)*/ = 0;
    virtual mw::Header::CPtr GetHeader(const mw::Hash& hash) const /*throw(NotFoundException)*/ = 0;

    virtual mw::HeaderAndPegs::CPtr GetHeaderAndPegs(const uint64_t height) const /*throw(NotFoundException)*/ = 0;
    virtual mw::HeaderAndPegs::CPtr GetHeaderAndPegs(const mw::Hash& hash) const /*throw(NotFoundException)*/ = 0;

    virtual mw::Block::CPtr GetBlock(const uint64_t height) const /*throw(NotFoundException)*/ = 0;
    virtual mw::Block::CPtr GetBlock(const mw::Hash& hash) const /*throw(NotFoundException)*/ = 0;
};

END_NAMESPACE