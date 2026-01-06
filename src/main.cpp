/*
** R-Type Map Editor
** Main entry point
*/
#include "map_editor.hpp"
#include "config_loader.hpp"
#include <iostream>
#include <filesystem>

using namespace rtype::editor;

int main() {
    // Try to load client.json from multiple locations
    std::string configPath;
    std::vector<std::string> candidatePaths = {
        "../assets/configs/client.json",           // From build directory
        "assets/configs/client.json",              // Current directory
        "./assets/configs/client.json",            // Explicit current
        "../../assets/configs/client.json",        // Two levels up
        "/home/hugo/R-Type/assets/configs/client.json"
    };
    
    for (const auto& path : candidatePaths) {
        if (std::filesystem::exists(path)) {
            configPath = path;
            std::cout << "Loaded config from: " << path << std::endl;
            break;
        }
    }
    
    AssetRegistry registry;
    if (!configPath.empty()) {
        registry = ConfigLoader::loadAssetsFromConfig(configPath);
        std::cout << "Loaded " << registry.size() << " assets from config" << std::endl;
    } else {
        std::cerr << "Warning: Could not load client.json, using defaults" << std::endl;
    }
    
    // Create and run editor
    MapEditor editor(1000, 700);
    editor.loadAssets(registry);
    editor.run();
    
    return 0;
}
