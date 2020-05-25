#include <mw/ltc/node/Context.h>

Context::CPtr Context::Create()
{
    auto pContext = new Context(
        std::make_unique<BlockFactory>(),
        std::make_unique<HeaderFactory>(),
        std::make_unique<KernelFactory>()
    );
    return std::shared_ptr<Context>(pContext);
}