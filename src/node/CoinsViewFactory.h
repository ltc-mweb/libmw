#pragma once

#include <mw/node/CoinsView.h>
#include <mw/models/block/Header.h>
#include <mw/models/tx/UTXO.h>
#include <mw/file/FilePath.h>
#include <mw/mmr/MMR.h>
#include <mw/db/IBlockStore.h>
#include <libmw/interfaces.h>
#include <functional>

class CoinsViewFactory
{
public:
    static mw::CoinsViewDB::Ptr CreateDBView(
        const std::shared_ptr<libmw::IDBWrapper>& pDBWrapper,
        const mw::IBlockStore& blockStore,
        const FilePath& chainDir,
        const mw::Hash& firstMWHeaderHash,
        const mw::Hash& stateHeaderHash,
        const std::vector<UTXO::CPtr>& utxos,
        const std::vector<Kernel>& kernels
    );

private:
    static mmr::MMR::Ptr BuildAndValidateKernelMMR(
        const mw::IBlockStore& blockStore,
        const FilePath& chainDir,
        const mw::Hash& firstMWHeaderHash,
        const mw::Header::CPtr& pStateHeader,
        const std::vector<Kernel>& kernels
    );

	static mmr::LeafSet::Ptr BuildAndValidateLeafSet(
		const FilePath& chainDir,
		const mw::Header::CPtr& pStateHeader,
		const std::vector<UTXO::CPtr>& utxos
	);

    static mmr::MMR::Ptr BuildAndValidateOutputMMR(
        const FilePath& chainDir,
        const mw::Header::CPtr& pStateHeader,
        const std::vector<UTXO::CPtr>& utxos
    );

	static mmr::MMR::Ptr BuildAndValidateRangeProofMMR(
		const FilePath& chainDir,
		const mw::Header::CPtr& pStateHeader,
		const std::vector<UTXO::CPtr>& utxos
	);
};