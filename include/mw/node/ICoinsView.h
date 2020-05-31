#pragma once

namespace mw
{
namespace ltc
{

//
// An interface for the various views of the extension block's UTXO set.
// This is similar to CCoinsView in the main codebase, and in fact, each CCoinsView
// should also hold an instance of an ICoinsView for use with mimblewimble-related logic.
//
class ICoinsView
{
public:

};

}
}