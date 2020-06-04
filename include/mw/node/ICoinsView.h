#pragma once

#include <mw/common/Macros.h>

MW_NAMESPACE

//
// An interface for the various views of the extension block's UTXO set.
// This is similar to CCoinsView in the main codebase, and in fact, each CCoinsView
// should also hold an instance of a mw::ICoinsView for use with mimblewimble-related logic.
//
class ICoinsView
{
public:
    using Ptr = std::shared_ptr<ICoinsView>;
};

END_NAMESPACE