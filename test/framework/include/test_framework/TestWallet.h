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
        : libmw::IWallet(), m_seed(std::move(seed)), m_nextPath({ 1, 1, 0 }), m_coins{}, m_depthInChain{}
    {

    }

    libmw::PrivateKey GenerateNewHDKey() final
    {
        KeyChainPath path = m_nextPath;
        m_nextPath = m_nextPath.GetNextSibling();
        return libmw::PrivateKey{ path.Format(), ToBlind(path.Format()).array() };
    }

    libmw::PrivateKey GetHDKey(const std::string& bip32Path) const final
    {
        return libmw::PrivateKey{ bip32Path, ToBlind(bip32Path).array() };
    }

    std::vector<libmw::Coin> ListCoins() const final
    {
        return m_coins;
    }

    void AddCoins(const std::vector<libmw::Coin>& coins) final
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

    void DeleteCoins(const std::vector<libmw::Coin>& coins) final
    {
        for (const libmw::Coin& coin : coins) {
            for (auto iter = m_coins.begin(); iter != m_coins.end(); iter++) {
                if (coin.commitment == iter->commitment) {
                    m_coins.erase(iter);
                    break;
                }
            }
        }
    }

    std::vector<libmw::Coin> SelectCoins(
        const std::vector<libmw::Coin>& coins,
        const uint64_t amount) const final
    {
        std::vector<libmw::Coin> selected_coins;

        uint64_t inputs_amount = 0;
        for (const libmw::Coin& coin : coins) {
            if (coin.spent) {
                continue;
            }

            inputs_amount += coin.amount;
            selected_coins.push_back(coin);
            if (inputs_amount >= amount) {
                break;
            }
        }

        if (inputs_amount < amount) {
            ThrowInsufficientFunds("Not enough funds");
        }

        return selected_coins;
    }

    uint64_t GetDepthInActiveChain(const libmw::BlockHash& canonical_block_hash) const final
    {
        auto iter = m_depthInChain.find(canonical_block_hash);
        if (iter != m_depthInChain.end()) {
            return iter->second;
        }

        return 0;
    }

    void SetDepthInActiveChain(const libmw::BlockHash& canonical_block_hash, const uint64_t depth)
    {
        auto iter = m_depthInChain.find(canonical_block_hash);
        if (iter != m_depthInChain.end()) {
            iter->second = depth;
        } else {
            m_depthInChain.insert({ canonical_block_hash, depth });
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
    std::map<libmw::BlockHash, uint64_t> m_depthInChain;
};