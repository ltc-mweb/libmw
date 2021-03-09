#include "CoinsViewFactory.h"

#include <mw/mmr/backends/FileBackend.h>
#include <mw/exceptions/ValidationException.h>
#include <mw/crypto/Bulletproofs.h>
#include <mw/crypto/Schnorr.h>
#include <mw/consensus/KernelSumValidator.h>
#include <mw/db/CoinDB.h>
#include <mw/db/MMRInfoDB.h>

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
    const std::vector<Kernel>& kernels)
{
	auto pStateHeader = blockStore.GetHeader(stateHeaderHash);
	assert(pStateHeader != nullptr);

    if (kernels.size() != pStateHeader->GetNumKernels()) {
        ThrowValidation(EConsensusError::MMR_MISMATCH);
    }

	auto pBatch = pDBWrapper->CreateBatch();

	MMRInfo mmr_info;

	MMRInfoDB mmr_db(pDBWrapper.get(), pBatch.get());
	auto pMMRInfo = mmr_db.GetLatest();
	if (pMMRInfo) {
		mmr_info = *pMMRInfo;
	}

	mmr_info.index++;
	mmr_info.pruned = stateHeaderHash;
	mmr_info.compacted = boost::none; // TODO: Should we just always compact to stateHeaderHash?

	auto pLeafSet = BuildAndValidateLeafSet(
		mmr_info.index,
		chainDir,
		pStateHeader,
		utxos
	);

    auto pKernelMMR = BuildAndValidateKernelMMR(
        pDBWrapper,
		mmr_info.index,
        blockStore,
        chainDir,
		firstMWHeaderHash,
        pStateHeader,
        kernels
    );

	auto pOutputMMR = BuildAndValidateOutputMMR(
        pDBWrapper,
		mmr_info.index,
		chainDir,
		pStateHeader,
		utxos
	);

	auto pRangeProofMMR = BuildAndValidateRangeProofMMR(
        pDBWrapper,
		mmr_info.index,
		chainDir,
		pStateHeader,
		utxos
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
		pOutputMMR,
		pRangeProofMMR
	);
}

// TODO: Also validate peg-in/peg-out transactions
mmr::MMR::Ptr CoinsViewFactory::BuildAndValidateKernelMMR(
    const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper,
	const uint32_t file_index,
	const mw::IBlockStore& blockStore,
    const FilePath& chainDir,
    const mw::Hash& firstMWHeaderHash,
    const mw::Header::CPtr& pStateHeader,
    const std::vector<Kernel>& kernels)
{
    auto mmrPath = chainDir.GetChild("kernels");
    mmr::MMR::Ptr pMMR = std::make_shared<mmr::MMR>(mmr::FileBackend::Open('K', mmrPath, file_index, pDBWrapper));

    auto pNextHeader = blockStore.GetHeader(firstMWHeaderHash);
	assert(pNextHeader != nullptr);

	uint64_t kernels_added = 0;
    for (const Kernel& kernel : kernels)
    {
        pMMR->Add(kernel);
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

mmr::LeafSet::Ptr CoinsViewFactory::BuildAndValidateLeafSet(
	const uint32_t file_index,
	const FilePath& chainDir,
	const mw::Header::CPtr& pStateHeader,
	const std::vector<UTXO::CPtr>& utxos)
{
	auto pLeafSet = mmr::LeafSet::Open(chainDir, file_index);
	for (const UTXO::CPtr& pUTXO : utxos)
	{
		pLeafSet->Add(pUTXO->GetLeafIndex());
	}

	if (pLeafSet->Root() != pStateHeader->GetLeafsetRoot()) {
		ThrowValidation(EConsensusError::MMR_MISMATCH);
	}

	return pLeafSet;
}

mmr::MMR::Ptr CoinsViewFactory::BuildAndValidateOutputMMR(
    const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper,
	const uint32_t file_index,
    const FilePath& chainDir,
    const mw::Header::CPtr& pStateHeader,
    const std::vector<UTXO::CPtr>& utxos)
{
    auto mmrPath = chainDir.GetChild("outputs");
    auto pBackend = mmr::FileBackend::Open('O', mmrPath, file_index, pDBWrapper);
    mmr::MMR::Ptr pMMR = std::make_shared<mmr::MMR>(pBackend);

	// TODO: Need parent hashes
    for (const UTXO::CPtr& pUTXO : utxos)
    {
        pBackend->AddLeaf(mmr::Leaf::Create(pUTXO->GetLeafIndex(), pUTXO->GetOutput().Serialized()));
    }

	if (pMMR->Root() != pStateHeader->GetOutputRoot()) {
		ThrowValidation(EConsensusError::MMR_MISMATCH);
	}

	return pMMR;
}

mmr::MMR::Ptr CoinsViewFactory::BuildAndValidateRangeProofMMR(
    const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper,
	const uint32_t file_index,
	const FilePath& chainDir,
	const mw::Header::CPtr& pStateHeader,
	const std::vector<UTXO::CPtr>& utxos)
{
    auto mmrPath = chainDir.GetChild("rangeproofs");
    auto pBackend = mmr::FileBackend::Open('R', mmrPath, file_index, pDBWrapper);
	mmr::MMR::Ptr pMMR = std::make_shared<mmr::MMR>(pBackend);

	std::vector<ProofData> proofs;

	// TODO: Need parent hashes
	for (const UTXO::CPtr& pUTXO : utxos)
	{
		pBackend->AddLeaf(mmr::Leaf::Create(pUTXO->GetLeafIndex(), pUTXO->GetRangeProof()->Serialized()));

		proofs.push_back(pUTXO->BuildProofData());
		if (proofs.size() >= PROOF_BATCH_SIZE) {
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

	if (pMMR->Root() != pStateHeader->GetRangeProofRoot()) {
		ThrowValidation(EConsensusError::MMR_MISMATCH);
	}

	return pMMR;
}