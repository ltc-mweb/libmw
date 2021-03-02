#pragma once

#include <libmw/interfaces/wallet_interface.h>
#include <mw/models/crypto/BlindingFactor.h>
#include <mw/models/wallet/KeyChainPath.h>
#include <mw/crypto/Hasher.h>
#include <mw/crypto/Random.h>
#include <mw/exceptions/InsufficientFundsException.h>
#include <map>

class TestWallet : public libmw::IWallet
{
public:
    using Ptr = std::shared_ptr<TestWallet>;

    static TestWallet::Ptr Create()
    {
        return std::make_shared<TestWallet>(Random::CSPRNG<32>());
    }

    TestWallet(BlindingFactor&& seed)
        : libmw::IWallet(), m_seed(std::move(seed)), m_nextPath({ 1, 1, 0 }), m_coins{}
    {

    }

    libmw::PrivateKey GetHDKey(const std::string& bip32Path) const final
    {
        return libmw::PrivateKey{ bip32Path, ToBlind(bip32Path).array() };
    }

    bool GetCoin(const libmw::Commitment& output_commit, libmw::Coin& coin_out) const final
    {
        for (const libmw::Coin& coin : m_coins) {
            if (coin.commitment == output_commit) {
                coin_out = coin;
                return true;
            }
        }

        return false;
    }

    void AddCoins(const std::vector<libmw::Coin>& coins)
    {
        for (const libmw::Coin& coin : coins) {
            bool replaced = false;
            for (auto iter = m_coins.begin(); iter != m_coins.end(); iter++) {
                if (coin.commitment == iter->commitment) {
                    *iter = coin;
                    replaced = true;
                    break;
                }
            }

            if (!replaced) {
                m_coins.push_back(coin);
            }
        }
    }

private:
    // Just uses a quick, insecure method for generating deterministic keys from a path.
    // Suitable for testing only. Do not use in a production environment.
    BlindingFactor ToBlind(const std::string& path) const
    {        
        return Hashed(Serializer().Append(m_seed).Append(path).vec());
    }

    BlindingFactor m_seed;
    KeyChainPath m_nextPath;
    std::vector<libmw::Coin> m_coins;
};