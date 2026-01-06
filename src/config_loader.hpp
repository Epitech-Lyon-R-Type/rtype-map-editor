/*
** R-Type Map Editor
** Config loader for client.json
*/
#pragma once
#include <string>
#include "asset_info.hpp"

namespace rtype::editor {

class ConfigLoader {
public:
    static AssetRegistry loadAssetsFromConfig(const std::string& configPath);
};

} // namespace rtype::editor
