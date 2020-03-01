#pragma once

#include <mw/ltc/models/block/Header.h>
#include <mw/ltc/models/block/Block.h>

class Node
{
public:
    Header::CPtr GetHeader(const uint64_t height) const;
    Block::CPtr GetBlock(const uint64_t height) const;

};