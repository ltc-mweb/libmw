#include <catch.hpp>

#include <mw/crypto/Crypto.h>
#include <mw/crypto/Random.h>

TEST_CASE("AggSig Interaction")
{
    Hash message = Random::CSPRNG<32>().GetBigInt();

    // Generate sender keypairs
    SecretKey secretKeySender = Random::CSPRNG<32>();
    PublicKey publicKeySender = Crypto::CalculatePublicKey(secretKeySender);
    SecretKey secretNonceSender = Crypto::GenerateSecureNonce();
    PublicKey publicNonceSender = Crypto::CalculatePublicKey(secretNonceSender);

    // Generate receiver keypairs
    SecretKey secretKeyReceiver = Random::CSPRNG<32>();
    PublicKey publicKeyReceiver = Crypto::CalculatePublicKey(secretKeyReceiver);
    SecretKey secretNonceReceiver = Crypto::GenerateSecureNonce();
    PublicKey publicNonceReceiver = Crypto::CalculatePublicKey(secretNonceReceiver);

    // Add pubKeys and pubNonces
    PublicKey sumPubKeys = Crypto::AddPublicKeys(
        std::vector<PublicKey>({ publicKeySender, publicKeyReceiver })
    );

    PublicKey sumPubNonces = Crypto::AddPublicKeys(
        std::vector<PublicKey>({ publicNonceSender, publicNonceReceiver })
    );

    // Generate partial signatures
    CompactSignature senderPartialSignature = Crypto::CalculatePartialSignature(
        secretKeySender,
        secretNonceSender,
        sumPubKeys,
        sumPubNonces,
        message
    );
    CompactSignature receiverPartialSignature = Crypto::CalculatePartialSignature(
        secretKeyReceiver,
        secretNonceReceiver,
        sumPubKeys,
        sumPubNonces,
        message
    );

    // Validate partial signatures
    const bool senderSigValid = Crypto::VerifyPartialSignature(
        senderPartialSignature,
        publicKeySender,
        sumPubKeys,
        sumPubNonces,
        message
    );
    REQUIRE(senderSigValid == true);

    const bool receiverSigValid = Crypto::VerifyPartialSignature(
        receiverPartialSignature,
        publicKeyReceiver,
        sumPubKeys,
        sumPubNonces,
        message
    );
    REQUIRE(receiverSigValid == true);

    // Aggregate signature and validate
    Signature aggregateSignature = Crypto::AggregateSignatures(
        std::vector<CompactSignature>({ senderPartialSignature, receiverPartialSignature }),
        sumPubNonces
    );
    const bool aggSigValid = Crypto::VerifyAggregateSignature(
        aggregateSignature,
        sumPubKeys,
        message
    );
    REQUIRE(aggSigValid == true);
}


TEST_CASE("Coinbase Signature")
{
    Hash message = Random::CSPRNG<32>().GetBigInt();
    SecretKey secretKey = Random::CSPRNG<32>();
    Commitment commitment = Crypto::CommitBlinded(0, BlindingFactor(secretKey));

    Signature signature = Crypto::BuildSignature(secretKey, message);

    const bool valid = Crypto::VerifyKernelSignatures({ &signature }, { &commitment }, { &message });
    REQUIRE(valid == true);
}