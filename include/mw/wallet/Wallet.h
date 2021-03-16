#pragma once

#include <libmw/interfaces/chain_interface.h>
#include <libmw/interfaces/wallet_interface.h>

#include <mw/models/block/Block.h>
#include <mw/models/tx/Transaction.h>
#include <mw/models/crypto/PublicKey.h>
#include <mw/models/wallet/PartialTx.h>
#include <mw/models/wallet/StealthAddress.h>

class Wallet
{
public:
    Wallet(const libmw::IWallet::Ptr& pWalletInterface, SecretKey&& scan_secret, SecretKey&& spend_secret)
        : m_pWalletInterface(pWalletInterface), m_scanSecret(std::move(scan_secret)), m_spendSecret(std::move(spend_secret)) { }

    static Wallet Open(const libmw::IWallet::Ptr& pWalletInterface);

    mw::Transaction::CPtr CreateTx(
        const std::vector<Commitment>& input_commits,
        const std::vector<std::pair<uint64_t, StealthAddress>>& recipients, // Includes pegins & change
        const std::vector<PegOutCoin>& pegouts,
        const boost::optional<uint64_t>& pegin_amount,
        const uint64_t fee
    ) const;

    StealthAddress GetStealthAddress(const uint32_t index) const;

    std::vector<libmw::Coin> GetCoins(const std::vector<Commitment>& commitments) const
    {
        std::vector<libmw::Coin> coins;
        for (const Commitment& commitment : commitments) {
            libmw::Coin coin;
            if (m_pWalletInterface->GetCoin(commitment.array(), coin)) {
                coins.push_back(std::move(coin));
            }
        }

        return coins;
    }

    libmw::IWallet::Ptr GetInterface() const noexcept { return m_pWalletInterface; }

    bool RewindOutput(const Output& output, libmw::Coin& coin) const;

private:
    SecretKey GetSpendKey(const uint32_t index) const;
    bool IsSpendPubKey(const PublicKey& spend_pubkey, uint32_t& index_out) const;

    libmw::IWallet::Ptr m_pWalletInterface;
    SecretKey m_scanSecret;
    SecretKey m_spendSecret;
};