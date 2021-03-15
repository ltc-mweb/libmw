#include <mw/wallet/Wallet.h>
#include <mw/crypto/Keys.h>
#include <mw/crypto/Random.h>
#include <mw/consensus/ChainParams.h>
#include <mw/exceptions/InsufficientFundsException.h>

#include "Transact.h"

Wallet Wallet::Open(const libmw::IWallet::Ptr& pWalletInterface)
{
    assert(pWalletInterface != nullptr);

    SecretKey scan_secret(pWalletInterface->GetHDKey("m/1/0/100'").keyBytes);
    SecretKey spend_secret(pWalletInterface->GetHDKey("m/1/0/101'").keyBytes);

    return Wallet(pWalletInterface, std::move(scan_secret), std::move(spend_secret));
}

mw::Transaction::CPtr Wallet::CreateTx(
    const std::vector<Commitment>& input_commits,
    const std::vector<std::pair<uint64_t, StealthAddress>>& recipients,
    const std::vector<PegOutCoin>& pegouts,
    const boost::optional<uint64_t>& pegin_amount,
    const uint64_t fee) const
{
    return Transact(*this).CreateTx(input_commits, recipients, pegouts, pegin_amount, fee);
}

StealthAddress Wallet::GetStealthAddress(const uint32_t index) const
{
    PublicKey Bi = PublicKey::From(GetSpendKey(index));
    PublicKey Ai = Bi.Mul(m_scanSecret); 

    return StealthAddress(Ai, Bi);
}

SecretKey Wallet::GetSpendKey(const uint32_t index) const
{
    SecretKey mi = Hasher(EHashTag::ADDRESS)
        .Append<uint32_t>(index)
        .Append(m_scanSecret)
        .hash();

    return Crypto::AddPrivateKeys(m_spendSecret, mi);
}

void Wallet::ScanForOutputs(const libmw::IChain::Ptr& pChain)
{
    // TODO: Just return outputs

    //std::vector<libmw::Coin> coins_found;

    //auto pChainIter = pChain->NewIterator();
    //while (pChainIter->Valid()) {
    //    try {
    //        libmw::BlockRef block_ref = pChainIter->GetBlock();
    //        if (block_ref.IsNull()) {
    //            // TODO: Use output mmr
    //        } else {
    //            for (const Output& output : block_ref.pBlock->GetOutputs()) {
    //                libmw::Coin coin;
    //                if (RewindOutput(output, coin)) {
    //                    coin.included_block = boost::make_optional<libmw::BlockHash>(pChainIter->GetCanonicalHash());
    //                    if (coin.address_index != libmw::CHANGE_INDEX && !Features(coin.features).IsSet(EOutputFeatures::PEGGED_IN)) {
    //                        // TODO: Create CWalletTx
    //                    }
    //                }
    //            }

    //            for (const Input& input : block_ref.pBlock->GetInputs()) {
    //                auto pCoinIter = coinmap.find(input.GetCommitment());
    //                if (pCoinIter != coinmap.end()) {
    //                    pCoinIter->second.spent = true;
    //                    pCoinIter->second.spent_block = pChainIter->GetCanonicalHash();
    //                }
    //            }
    //        }
    //    } catch (std::exception&) {}

    //    pChainIter->Next();
    //}
}

bool Wallet::RewindOutput(const Output& output, libmw::Coin& coin) const
{
    SecretKey a(m_pWalletInterface->GetHDKey("m/1/0/100'").keyBytes);

    SecretKey t = Hashed(EHashTag::DERIVE, output.Ke().Mul(a));
    if (t[0] == output.GetViewTag()) {
        PublicKey B = output.Ko().Sub(Hashed(EHashTag::OUT_KEY, t));

        // Check if B belongs to wallet
        uint32_t index = 0;
        if (m_pWalletInterface->IsMine(B.array(), index)) {
            StealthAddress wallet_addr = GetStealthAddress(index);
            Deserializer hash64(Hash512(t).vec());
            SecretKey r = hash64.Read<SecretKey>();
            uint64_t value = output.GetMaskedValue() ^ hash64.Read<uint64_t>();
            BigInt<16> n = output.GetMaskedNonce() ^ hash64.ReadVector(16);

            if (Commitment::Switch(r, value) == output.GetCommitment()) {
                // Calculate Carol's sending key 's' and check that s*B ?= Ke
                SecretKey s = Hasher(EHashTag::SEND_KEY)
                    .Append(wallet_addr.A())
                    .Append(wallet_addr.B())
                    .Append(value)
                    .Append(n)
                    .hash();
                if (output.Ke() == wallet_addr.B().Mul(s)) {
                    SecretKey private_key = Crypto::AddPrivateKeys(
                        Hashed(EHashTag::OUT_KEY, t),
                        GetSpendKey(index)
                    );
                    coin = libmw::Coin{
                        output.GetFeatures().Get(),
                        index,
                        private_key.array(),
                        r.array(),
                        value,
                        output.GetCommitment().array()
                    };
                    return true;
                }
            }
        }
    }

    return false;
}