#include "CoinsViewFactory.h"

#include <mw/crypto/Bulletproofs.h>
#include <mw/crypto/Schnorr.h>
#include <mw/consensus/KernelSumValidator.h>
#include <mw/db/CoinDB.h>
#include <mw/db/LeafDB.h>
#include <mw/db/MMRInfoDB.h>
#include <mw/exceptions/ValidationException.h>
#include <mw/mmr/backends/FileBackend.h>
#include <mw/mmr/MMRFactory.h>
#include <mw/mmr/MMRUtil.h>

static const size_t KERNEL_BATCH_SIZE = 512;
static const size_t PROOF_BATCH_SIZE = 512;

// TODO: Use StateValidator
mw::CoinsViewDB::Ptr CoinsViewFactory::CreateDBView(
	const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper,
	const mw::IBlockStore& blockStore,
    const FilePath& chainDir,
	const mw::Hash& firstMWHeaderHash,
	const mw::Hash& stateHeaderHash,
    const std::vector<UTXO::CPtr>& utxos,
    const std::vector<Kernel>& kernels,
	const BitSet& leafset,
	const std::vector<mw::Hash>& pruned_parent_hashes)
{
	auto pStateHeader = blockStore.GetHeader(stateHeaderHash);
	assert(pStateHeader != nullptr);

    if (kernels.size() != pStateHeader->GetNumKernels()) {
        ThrowValidation(EConsensusError::MMR_MISMATCH);
    }

	/**
	 * Update MMRInfo
	 */
	auto pMMRInfo = MMRInfoDB(pDBWrapper.get()).GetLatest();
	MMRInfo mmr_info = pMMRInfo ? *pMMRInfo : MMRInfo{};
	mmr_info.index++;
	mmr_info.pruned = stateHeaderHash;
	mmr_info.compact_index++;
	mmr_info.compacted = stateHeaderHash;
	MMRInfoDB(pDBWrapper.get()).Save(mmr_info);


	/**
	 * Build LeafSet
	 */
	File(mmr::LeafSet::GetPath(chainDir, mmr_info.index))
		.Write(leafset.bytes());
	auto pLeafSet = mmr::LeafSet::Open(chainDir, mmr_info.index);
	if (pLeafSet->Root() != pStateHeader->GetLeafsetRoot()) {
		ThrowValidation(EConsensusError::MMR_MISMATCH);
	}

	/**
	 * Build PruneList
	 */
	BitSet compact_bitset = mmr::MMRUtil::BuildCompactBitSet(pStateHeader->GetNumTXOs(), leafset.bitset);
	File(mmr::PruneList::GetPath(chainDir, mmr_info.compact_index))
		.Write(compact_bitset.bytes());
	auto pPruneList = mmr::PruneList::Open(chainDir, mmr_info.compact_index);

	/**
	 * Build KernelMMR
	 */
	auto pBatch = pDBWrapper->CreateBatch();
    auto pKernelMMR = BuildAndValidateKernelMMR(
        pDBWrapper,
		pBatch,
		mmr_info,
        blockStore,
        chainDir,
		firstMWHeaderHash,
        pStateHeader,
        kernels
    );
	
	/**
	 * Build Output PMMR
	 */
	auto pOutputPMMR = BuildAndValidateOutputMMR(
		pDBWrapper,
		pBatch,
		mmr_info,
		chainDir,
		pStateHeader,
		utxos,
		leafset,
		pPruneList,
		pruned_parent_hashes
	);

	/**
	 * Build RangeProof PMMR
	 */
	auto pRangeProofMMR = BuildAndValidateRangeProofMMR(
        pDBWrapper,
		pBatch,
		mmr_info,
		chainDir,
		pStateHeader,
		utxos,
		leafset,
		pPruneList,
		pruned_parent_hashes
	);

    std::vector<Commitment> utxo_commitments;
    std::transform(
        utxos.cbegin(), utxos.cend(),
        std::back_inserter(utxo_commitments),
        [](const UTXO::CPtr& pUTXO) { return pUTXO->GetCommitment(); }
    );

	// Block sum validation
	KernelSumValidator::ValidateState(
		utxo_commitments,
		kernels,
		pStateHeader->GetKernelOffset()
	);

	// Add UTXOs to database
	CoinDB coinDB(pDBWrapper.get(), pBatch.get());
	coinDB.AddUTXOs(utxos);
	pBatch->Commit();

	return std::make_shared<mw::CoinsViewDB>(
		pStateHeader,
		pDBWrapper,
		pLeafSet,
		pKernelMMR,
		pOutputPMMR,
		pRangeProofMMR
	);
}

// TODO: Also validate peg-in/peg-out transactions
mmr::MMR::Ptr CoinsViewFactory::BuildAndValidateKernelMMR(
    const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper,
	const std::unique_ptr<libmw::IDBBatch>& pBatch,
	const MMRInfo& mmr_info,
	const mw::IBlockStore& blockStore,
    const FilePath& chainDir,
    const mw::Hash& firstMWHeaderHash,
    const mw::Header::CPtr& pStateHeader,
    const std::vector<Kernel>& kernels)
{
    mmr::MMR::Ptr pMMR = std::make_shared<mmr::MMR>(
		mmr::FileBackend::Open('K', chainDir, mmr_info.index, pDBWrapper, nullptr)
	);

    auto pNextHeader = blockStore.GetHeader(firstMWHeaderHash);
	assert(pNextHeader != nullptr);

	std::vector<mmr::Leaf> leaves;
	leaves.reserve(kernels.size());

	uint64_t kernels_added = 0;
    for (const Kernel& kernel : kernels)
    {
        mmr::LeafIndex leaf_idx = pMMR->Add(kernel);
		leaves.push_back(mmr::Leaf::Create(leaf_idx, kernel.Serialized()));

		++kernels_added;

        // We have to loop here because some blocks may not have any new kernels.
        while (kernels_added == pNextHeader->GetNumKernels())
        {
            if (pNextHeader->GetKernelRoot() != pMMR->Root()) {
                ThrowValidation(EConsensusError::MMR_MISMATCH);
            }

            if (pNextHeader == pStateHeader) {
                break;
            }

			pNextHeader = blockStore.GetHeader(pNextHeader->GetHeight() + 1);
        }
    }

	LeafDB('K', pDBWrapper.get(), pBatch.get())
		.Add(leaves);

	// Verify kernel signatures
	std::vector<SignedMessage> signatures;
	for (const Kernel& kernel : kernels)
	{
		PublicKey pubkey = Crypto::ToPublicKey(kernel.GetCommitment());
		signatures.push_back({ kernel.GetSignatureMessage(), std::move(pubkey), kernel.GetSignature() });

		if (signatures.size() >= KERNEL_BATCH_SIZE) {
			if (!Schnorr::BatchVerify(signatures)) {
				ThrowValidation(EConsensusError::INVALID_SIG);
			}

			signatures.clear();
		}
	}

	if (!signatures.empty()) {
		if (!Schnorr::BatchVerify(signatures)) {
			ThrowValidation(EConsensusError::INVALID_SIG);
		}
	}

    return pMMR;
}

mmr::MMR::Ptr CoinsViewFactory::BuildAndValidateOutputMMR(
    const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper,
	const std::unique_ptr<libmw::IDBBatch>& pBatch,
	const MMRInfo& mmr_info,
    const FilePath& chainDir,
    const mw::Header::CPtr& pStateHeader,
    const std::vector<UTXO::CPtr>& utxos,
	const BitSet& leafset,
	const mmr::PruneList::Ptr& pPruneList,
	const std::vector<mw::Hash>& pruned_parent_hashes)
{
	if (leafset.count() != utxos.size()) {
		ThrowValidation(EConsensusError::MMR_MISMATCH);
	}

	std::vector<mmr::Leaf> output_leaves;
	size_t utxo_idx = 0;
	for (uint64_t i = 0; i < leafset.size(); i++) {
		if (leafset.test(i)) {
			const UTXO::CPtr& pUTXO = utxos[utxo_idx++];
			output_leaves.push_back(
				mmr::Leaf::Create(mmr::LeafIndex::At(i), pUTXO->ToOutputId().Serialized())
			);
		}
	}

	mmr::MMR::Ptr pOutputPMMR = mmr::MMRFactory::Build(
		'O',
		pDBWrapper,
		pBatch,
		pPruneList,
		mmr_info,
		chainDir,
		leafset,
		output_leaves,
		pruned_parent_hashes
	);

	if (pOutputPMMR->Root() != pStateHeader->GetOutputRoot()
		|| pOutputPMMR->GetNumLeaves() != pStateHeader->GetNumTXOs()) {
		ThrowValidation(EConsensusError::MMR_MISMATCH);
	}

	return pOutputPMMR;
}

mmr::MMR::Ptr CoinsViewFactory::BuildAndValidateRangeProofMMR(
    const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper,
	const std::unique_ptr<libmw::IDBBatch>& pBatch,
	const MMRInfo& mmr_info,
	const FilePath& chainDir,
	const mw::Header::CPtr& pStateHeader,
	const std::vector<UTXO::CPtr>& utxos,
	const BitSet& leafset,
	const mmr::PruneList::Ptr& pPruneList,
	const std::vector<mw::Hash>& pruned_parent_hashes)
{
	std::vector<ProofData> proofs;
	proofs.reserve(PROOF_BATCH_SIZE);

	std::vector<mmr::Leaf> leaves;
	leaves.reserve(utxos.size());

	for (const UTXO::CPtr& pUTXO : utxos) {
		leaves.push_back(
			mmr::Leaf::Create(pUTXO->GetLeafIndex(), pUTXO->GetRangeProof()->Serialized())
		);

		proofs.push_back(pUTXO->BuildProofData());
		if (proofs.size() == PROOF_BATCH_SIZE) {
			if (!Bulletproofs::BatchVerify(proofs)) {
				ThrowValidation(EConsensusError::BULLETPROOF);
			}

			proofs.clear();
		}
	}

	if (!proofs.empty()) {
		if (!Bulletproofs::BatchVerify(proofs)) {
			ThrowValidation(EConsensusError::BULLETPROOF);
		}
	}

	mmr::MMR::Ptr pRangeProofPMMR = mmr::MMRFactory::Build(
		'R',
		pDBWrapper,
		pBatch,
		pPruneList,
		mmr_info,
		chainDir,
		leafset,
		leaves,
		pruned_parent_hashes
	);

	if (pRangeProofPMMR->Root() != pStateHeader->GetRangeProofRoot()
		|| pRangeProofPMMR->GetNumLeaves() != pStateHeader->GetNumTXOs()) {
		ThrowValidation(EConsensusError::MMR_MISMATCH);
	}

	if (pRangeProofPMMR->Root() != pStateHeader->GetRangeProofRoot()) {
		ThrowValidation(EConsensusError::MMR_MISMATCH);
	}

	return pRangeProofPMMR;
}