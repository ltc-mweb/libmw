

/* TEST:
 *      1. Create a RangeProof via Crypto::GenerateRangeProof. Use the same value for privateNonce and rewindNonce.
 *      2. Try rewinding it via Crypto::RewindRangeProof using the *wrong* rewindNonce. Make sure it returns null.
 *      3. Try rewinding it via Crypto::RewindRangeProof using the *correct* rewindNonce (the one you passed to GenerateRangeProof). Make sure it returns a valid RewoundProof.
 *      4. Make sure amount, blindingFactor (aka 'key'), and ProofMessage match the values passed to GenerateRangeProof
 *      5. Make sure VerifyRangeProofs returns true. Use an empty vector for the third tuple value.
 */