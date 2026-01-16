/*
** R-Type Map Editor
** Asset information from client.json
*/
#pragma once
#include <string>
#include <unordered_map>

namespace rtype::editor {

struct AssetInfo {
    std::string spritePath;
    int width = 32;
    int height = 32;
};

using AssetRegistry = std::unordered_map<std::string, AssetInfo>;

}
