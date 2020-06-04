#pragma once

#include "Context.h"

#include <mw/models/crypto/Commitment.h>
#include <mw/models/crypto/SecretKey.h>
#include <mw/models/crypto/Signature.h>
#include <mw/models/crypto/PublicKey.h>
#include <mw/models/crypto/Hash.h>

#include <shared_mutex>

class AggSig
{
public:
    AggSig(Locked<Context>& context) : m_context(context) { }
    ~AggSig() = default;

    SecretKey GenerateSecureNonce() const;

    //
    // Signs the message hash with the given key.
    //
    Signature SignMessage(
        const SecretKey& secretKey,
        const Hash& message
    );

    CompactSignature CalculatePartialSignature(
        const SecretKey& secretKey,
        const SecretKey& secretNonce,
        const PublicKey& sumPubKeys,
        const PublicKey& sumPubNonces,
        const Hash& message
    );

    bool VerifyPartialSignature(
        const CompactSignature& partialSignature,
        const PublicKey& publicKey,
        const PublicKey& sumPubKeys,
        const PublicKey& sumPubNonces,
        const Hash& message
    ) const;

    Signature AggregateSignatures(
        const std::vector<CompactSignature>& signatures,
        const PublicKey& sumPubNonces
    ) const;

    bool VerifyAggregateSignatures(
        const std::vector<const Signature*>& signatures,
        const std::vector<const Commitment*>& publicKeys,
        const std::vector<const Hash*>& messages
    ) const;

    bool VerifyAggregateSignature(
        const Signature& signature,
        const PublicKey& sumPubKeys,
        const Hash& message
    ) const;

private:
    Locked<Context> m_context;
};