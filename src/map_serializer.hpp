/*
** R-Type Map Editor
** Map serialization to/from JSON
*/
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "entity_data.hpp"
#include "asset_info.hpp"

namespace rtype::editor {
struct MapData {
    std::string gameConfigPath = "./assets/configs/rtype.json";
    int id = 1;
    float scrollSpeed = 2.0F;
    int width = 800;
    int height = 600;
    std::vector<EntityData> entities;
    AssetRegistry assets;
};

class MapSerializer {
public:
    static bool saveMapToFile(const std::string& filename, const MapData& map);
    static MapData loadMapFromFile(const std::string& filename);
    static std::string mapToJson(const MapData& map);
    static MapData jsonToMap(const std::string& jsonStr);
private:
    static std::unordered_map<std::string, int> loadTypeRefs(const std::string& gameConfigPath);
    static std::unordered_map<int, std::string> invertTypeRefs(const std::unordered_map<std::string, int>& refs);
};
}
