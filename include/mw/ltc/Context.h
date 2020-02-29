#pragma once

#include <mw/core/Context.h>
#include <mw/ltc/models/block/Header.h>
#include <mw/ltc/models/block/Block.h>
#include <mw/ltc/models/tx/Kernel.h>

class BlockFactory : public IBlockFactory
{
public:
    virtual ~BlockFactory() = default;

    std::shared_ptr<const IBlock> Deserialize(Deserializer& deserializer) const final
    {
        // TODO: Implement
        return nullptr;
    }

    std::shared_ptr<const IBlock> FromJSON(const Json& json) const final
    {
        // TODO: Implement
        return nullptr;
    }
};

class HeaderFactory : public IHeaderFactory
{
public:
    virtual ~HeaderFactory() = default;

    std::shared_ptr<const IHeader> Deserialize(Deserializer& deserializer) const final
    {
        return Header::Deserialize(deserializer);
    }

    std::shared_ptr<const IHeader> FromJSON(const Json& json) const final
    {
        return Header::FromJSON(json);
    }
};

class KernelFactory : public IKernelFactory
{
public:
    virtual ~KernelFactory() = default;

    std::shared_ptr<const IKernel> Deserialize(Deserializer& deserializer) const final
    {
        return Kernel::Deserialize(deserializer);
    }

    std::shared_ptr<const IKernel> FromJSON(const Json& json) const final
    {
        return Kernel::FromJSON(json);
    }
};

Context::CPtr Context::Create()
{
    auto pContext = new Context(
        std::make_unique<BlockFactory>(),
        std::make_unique<HeaderFactory>(),
        std::make_unique<KernelFactory>()
    );
    return std::shared_ptr<Context>(pContext);
}