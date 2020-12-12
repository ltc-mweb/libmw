#pragma once

#include <mw/models/crypto/Commitment.h>
#include <mw/models/crypto/SecretKey.h>
#include <mw/models/crypto/Signature.h>
#include <mw/models/crypto/PublicKey.h>
#include <mw/models/crypto/Hash.h>

class Schnorr
{
public:
    //
    // Signs the message hash with the given key.
    //
    static Signature Sign(
        const uint8_t* secretKey,
        const mw::Hash& message
    );

    static bool Verify(
        const Signature& signature,
        const PublicKey& sumPubKeys,
        const mw::Hash& message
    );

    static bool BatchVerify(
        const std::vector<std::tuple<Signature, Commitment, mw::Hash>>& signatures
    );

private:
    static bool BatchVerify(
        const std::vector<const Signature*>& signatures,
        const std::vector<const Commitment*>& publicKeys,
        const std::vector<const mw::Hash*>& messages
    );
};