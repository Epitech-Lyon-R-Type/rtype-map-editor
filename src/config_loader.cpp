/*
** R-Type Map Editor
** Config loader implementation
*/
#include "config_loader.hpp"
#include "json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace rtype::editor {

AssetRegistry ConfigLoader::loadAssetsFromConfig(const std::string& configPath) {
    AssetRegistry registry;
    
    try {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            std::cerr << "Failed to open config: " << configPath << std::endl;
            return registry;
        }
        
        json j;
        file >> j;
        
        // Load sprites paths
        if (j.contains("sprites") && j["sprites"].is_object()) {
            for (auto& [key, value] : j["sprites"].items()) {
                AssetInfo info;
                info.spritePath = value.get<std::string>();
                registry[key] = info;
            }
        }
        
        // Load render rectangles (dimensions)
        if (j.contains("entities") && j["entities"].is_object()) {
            for (auto& [key, entity] : j["entities"].items()) {
                if (entity.contains("render") && entity["render"].is_object()) {
                    auto& render = entity["render"];
                    if (render.contains("rect") && render["rect"].is_object()) {
                        auto& rect = render["rect"];
                        if (registry.find(key) != registry.end()) {
                            if (rect.contains("w")) registry[key].width = rect["w"].get<int>();
                            if (rect.contains("h")) registry[key].height = rect["h"].get<int>();
                        }
                    }
                }
            }
        }
        
        std::cout << "Loaded " << registry.size() << " assets from config" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
    }
    
    return registry;
}

} // namespace rtype::editor
