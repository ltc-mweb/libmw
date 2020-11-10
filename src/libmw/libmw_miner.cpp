#include <libmw/node.h>

#include "Transformers.h"
#include "BlockStoreWrapper.h"

#include <mw/node/BlockBuilder.h>

LIBMW_NAMESPACE
MINER_NAMESPACE

MWEXPORT libmw::BlockBuilderRef NewBuilder(const uint64_t height, const libmw::CoinsViewRef& view)
{
    return libmw::BlockBuilderRef{ std::make_shared<mw::BlockBuilder>(height, view.pCoinsView) };
}

MWEXPORT uint8_t AddTransaction(
    const libmw::BlockBuilderRef& builder,
    const libmw::TxRef& transaction,
    const std::vector<libmw::PegIn>& pegins)
{
    assert(builder.pBuilder != nullptr);
    assert(transaction.pTransaction != nullptr);

    try {
        const bool success = builder.pBuilder->AddTransaction(transaction.pTransaction, TransformPegIns(pegins));
        return success ? 0 : 1;
    }
    catch (std::exception& e) {
        std::cout << "Failed to add transaction. " << e.what() << std::endl;
    }

    return 1;
}

MWEXPORT libmw::BlockRef BuildBlock(const libmw::BlockBuilderRef& builder)
{
    return libmw::BlockRef{ builder.pBuilder->BuildBlock() };
}

END_NAMESPACE // miner
END_NAMESPACE // libmw