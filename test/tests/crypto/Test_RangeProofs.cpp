#include <catch.hpp>

#include <mw/crypto/Crypto.h>
#include <mw/crypto/Random.h>

TEST_CASE("Range Proofs")
{
    const uint64_t value = 123;
    BlindingFactor blind = Random::CSPRNG<32>();
    Commitment commit = Crypto::CommitBlinded(value, blind);
    SecretKey nonce = Random::CSPRNG<32>();
    SecretKey nonce2 = Random::CSPRNG<32>();
    ProofMessage message = BigInt(Random::CSPRNG<20>().GetBigInt());

    // Create a RangeProof via Crypto::GenerateRangeProof. Use the same value for privateNonce and rewindNonce.
    RangeProof::CPtr pRangeProof = Crypto::GenerateRangeProof(
        value,
        BlindingFactor(blind).ToSecretKey(),
        nonce,
        nonce,
        message
    );

    // Try rewinding it via Crypto::RewindRangeProof using the *wrong* rewindNonce. Make sure it returns null.
    REQUIRE_FALSE(Crypto::RewindRangeProof(commit, *pRangeProof, nonce2));

    // Try rewinding it via Crypto::RewindRangeProof using the *correct* rewindNonce. Make sure it returns a valid RewoundProof.
    std::unique_ptr<RewoundProof> pRewoundProof = Crypto::RewindRangeProof(commit, *pRangeProof, nonce);
    REQUIRE(pRewoundProof);

    // Make sure amount, blindingFactor (aka 'key'), and ProofMessage match the values passed to GenerateRangeProof
    REQUIRE(*pRewoundProof == RewoundProof(
        value,
        std::make_unique<SecretKey>(BlindingFactor(blind).ToSecretKey()),
        ProofMessage(message)
    ));

    // Make sure VerifyRangeProofs returns true. Use an empty vector for the third tuple value.
    std::vector<std::tuple<Commitment, RangeProof::CPtr, std::vector<uint8_t>>> rangeProofs;
    rangeProofs.push_back(std::make_tuple(commit, pRangeProof, std::vector<uint8_t>()));
    REQUIRE(Crypto::VerifyRangeProofs(rangeProofs));
}