#pragma once

#include "Context.h"

#include <mw/models/crypto/Commitment.h>
#include <mw/models/crypto/SecretKey.h>
#include <mw/models/crypto/Signature.h>
#include <mw/models/crypto/PublicKey.h>
#include <mw/models/crypto/Hash.h>

class Schnorr
{
public:
    Schnorr(Locked<Context>& context) : m_context(context) { }
    ~Schnorr() = default;

    //
    // Signs the message hash with the given key.
    //
    Signature Sign(
        const SecretKey& secretKey,
        const Hash& message
    );

    bool Verify(
        const Signature& signature,
        const PublicKey& sumPubKeys,
        const Hash& message
    ) const;

    bool BatchVerify(
        const std::vector<const Signature*>& signatures,
        const std::vector<const Commitment*>& publicKeys,
        const std::vector<const Hash*>& messages
    ) const;

private:
    Locked<Context> m_context;
};