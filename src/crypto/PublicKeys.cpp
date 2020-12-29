#include "PublicKeys.h"
#include "ConversionUtil.h"

#include <mw/exceptions/CryptoException.h>
#include <mw/util/VectorUtil.h>

PublicKey PublicKeys::CalculatePublicKey(const BigInt<32>& privateKey) const
{
    const int verifyResult = secp256k1_ec_seckey_verify(m_context.Read()->Get(), privateKey.data());
    if (verifyResult != 1)
    {
        ThrowCrypto("Failed to verify secret key");
    }

    secp256k1_pubkey pubkey;
    const int createResult = secp256k1_ec_pubkey_create(
        m_context.Read()->Get(),
        &pubkey,
        privateKey.data()
    );
    if (createResult != 1)
    {
        ThrowCrypto("Failed to calculate public key");
    }

    return ConversionUtil(m_context).ToPublicKey(pubkey);
}

PublicKey PublicKeys::PublicKeySum(const std::vector<PublicKey>& publicKeys) const
{
    std::vector<secp256k1_pubkey> pubkeys = ConversionUtil(m_context).ToSecp256k1(publicKeys);
    std::vector<secp256k1_pubkey*> pubkeyPtrs = VectorUtil::ToPointerVec(pubkeys);

    secp256k1_pubkey pubkey;
    const int pubKeysCombined = secp256k1_ec_pubkey_combine(
        m_context.Read()->Get(),
        &pubkey,
        pubkeyPtrs.data(),
        pubkeyPtrs.size()
    );

    if (pubKeysCombined != 1)
    {
        ThrowCrypto("Failed to combine public keys.");
    }

    return ConversionUtil(m_context).ToPublicKey(pubkey);
}