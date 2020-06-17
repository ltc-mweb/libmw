#pragma once

#include "Context.h"

#include <mw/models/crypto/SecretKey.h>
#include <mw/models/crypto/PublicKey.h>

class PublicKeys
{
public:
    PublicKeys(Locked<Context>& context) : m_context(context) { }
    ~PublicKeys() = default;

    PublicKey CalculatePublicKey(const SecretKey& privateKey) const;
    PublicKey PublicKeySum(const std::vector<PublicKey>& publicKeys) const;

private:
    Locked<Context> m_context;
};