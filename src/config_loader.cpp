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
        
        // Load render rectangles (dimensions) from entities
        // Only add entities that are placeable (not projectiles, not UI elements)
        if (j.contains("entities") && j["entities"].is_object()) {
            for (auto& [entityName, entity] : j["entities"].items()) {
                // Skip non-placeable entities (missiles, camera, world, background)
                if (entityName.find("MISSILE") != std::string::npos ||
                    entityName.find("CAMERA") != std::string::npos ||
                    entityName.find("WORLD") != std::string::npos ||
                    entityName.find("BACKGROUND") != std::string::npos) {
                    continue;
                }
                
                if (entity.contains("render") && entity["render"].is_object()) {
                    auto& render = entity["render"];
                    if (render.contains("rect") && render["rect"].is_object()) {
                        auto& rect = render["rect"];
                        
                        //get the sprite key
                        std::string spriteKey = entityName;
                        if (render.contains("key") && render["key"].is_string()) {
                            spriteKey = render["key"].get<std::string>();
                        }
                        
                        if (registry.find(spriteKey) != registry.end()) {
                            int w = rect.contains("w") ? rect["w"].get<int>() : registry[spriteKey].width;
                            int h = rect.contains("h") ? rect["h"].get<int>() : registry[spriteKey].height;
                            registry[spriteKey].width = std::max(registry[spriteKey].width, w);
                            registry[spriteKey].height = std::max(registry[spriteKey].height, h);
                        }
                        
                        if (entityName != spriteKey && registry.find(spriteKey) != registry.end()) {
                            AssetInfo info = registry[spriteKey];
                            if (rect.contains("w")) info.width = rect["w"].get<int>();
                            if (rect.contains("h")) info.height = rect["h"].get<int>();
                            registry[entityName] = info;
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

}
