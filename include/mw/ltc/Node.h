#pragma once

#include <mw/ltc/models/block/Header.h>

class Node
{
public:
    Header::CPtr GetHeader(const uint64_t height) const;
};