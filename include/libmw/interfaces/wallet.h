#pragma once

#include <libmw/defs.h>

LIBMW_NAMESPACE
//
//class IKeyChain
//{
//public:
//    using Ptr = std::shared_ptr<libmw::IKeyChain>;
//
//    virtual ~IKeyChain() = default;
//
//    //
//    // Generates a new key, and saves it to the wallet database.
//    //
//    virtual libmw::PrivateKey GenerateNewKey() = 0;
//
//    //
//    // Calculates the private key at the given bip32 path.
//    // The key is NOT written to the database.
//    // @param bip32Path - eg. "m/44'/0/1/2"
//    //
//    virtual libmw::PrivateKey GetHDKey(const std::string& bip32Path) const = 0;
//};

class IWallet
{
public:
    using Ptr = std::shared_ptr<libmw::IWallet>;

    virtual ~IWallet() = default;

    //
    // Generates a new key, and saves it to the wallet database.
    //
    virtual libmw::PrivateKey GenerateNewKey() = 0;

    //
    // Calculates the private key at the given bip32 path.
    // The key is NOT written to the database.
    // @param bip32Path - eg. "m/44'/0/1/2"
    //
    virtual libmw::PrivateKey GetHDKey(const std::string& bip32Path) const = 0;

    virtual std::vector<libmw::Coin> ListCoins() const = 0;
    virtual void AddCoins(const std::vector<libmw::Coin>& coins) = 0;
    virtual void DeleteCoins(const std::vector<libmw::Coin>& coins) = 0;
};

END_NAMESPACE