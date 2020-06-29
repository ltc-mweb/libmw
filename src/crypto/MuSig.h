#pragma once

#include "Context.h"

#include <mw/models/crypto/Commitment.h>
#include <mw/models/crypto/SecretKey.h>
#include <mw/models/crypto/Signature.h>
#include <mw/models/crypto/PublicKey.h>
#include <mw/models/crypto/Hash.h>

class MuSig
{
public:
    MuSig(Locked<Context>& context) : m_context(context) { }
    ~MuSig() = default;

    SecretKey GenerateSecureNonce() const;

    CompactSignature CalculatePartialSignature(
        const SecretKey& secretKey,
        const SecretKey& secretNonce,
        const PublicKey& sumPubKeys,
        const PublicKey& sumPubNonces,
        const mw::Hash& message
    );

    bool VerifyPartialSignature(
        const CompactSignature& partialSignature,
        const PublicKey& publicKey,
        const PublicKey& sumPubKeys,
        const PublicKey& sumPubNonces,
        const mw::Hash& message
    ) const;

    Signature AggregateSignatures(
        const std::vector<CompactSignature>& signatures,
        const PublicKey& sumPubNonces
    ) const;

private:
    Locked<Context> m_context;
};