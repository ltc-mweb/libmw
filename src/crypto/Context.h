#pragma once

#include <mw/common/Lock.h>
#include <mw/crypto/Random.h>
#include <mw/models/crypto/SecretKey.h>
#include <mw/exceptions/CryptoException.h>
#include <mw/crypto/secp256k1.h>

class Context
{
public:
    Context()
    {
        m_pContext = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    }

    ~Context()
    {
        secp256k1_context_destroy(m_pContext);
    }

    secp256k1_context* Randomized()
    {
        const SecretKey randomSeed = Random::CSPRNG<32>();
        const int randomizeResult = secp256k1_context_randomize(m_pContext, randomSeed.data());
        if (randomizeResult != 1)
        {
            ThrowCrypto("Context randomization failed.");
        }

        return m_pContext;
    }

    secp256k1_context* Get() { return m_pContext; }
    const secp256k1_context* Get() const { return m_pContext; }

private:
    secp256k1_context* m_pContext;
};