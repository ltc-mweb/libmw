#pragma once

#include <mw/config/BaseConfig.h>
#include <mw/file/FilePath.h>

class NodeConfig : public BaseConfig
{
public:
    using Ptr = std::shared_ptr<NodeConfig>;

    static NodeConfig::Ptr Create(const FilePath& datadir, std::unordered_map<std::string, std::string>&& options)
    {
        auto mwdir = datadir.GetChild("mw");
        mwdir.GetChild("chain").CreateDirIfMissing();

        return std::shared_ptr<NodeConfig>(new NodeConfig{ mwdir, std::move(options) });
    }

    //
    // Returns the root data folder for the node.
    //
    const FilePath& GetDataDir() const noexcept { return m_datadir; }

    FilePath GetChainDir() const { return m_datadir.GetChild("chain"); }

private:
    NodeConfig(const FilePath& datadir, std::unordered_map<std::string, std::string>&& options)
        : BaseConfig(std::move(options)), m_datadir(datadir) { }

    FilePath m_datadir;
};