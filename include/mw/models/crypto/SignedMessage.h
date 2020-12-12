#pragma once

#include <mw/models/crypto/Hash.h>
#include <mw/models/crypto/PublicKey.h>
#include <mw/models/crypto/Signature.h>

/// <summary>
/// Contains a hashed message, a signature of that message, and the public key it was signed for.
/// </summary>
struct SignedMessage
{
    mw::Hash message_hash;
    PublicKey public_key;
    Signature signature;
};