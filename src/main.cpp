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
    std::string configPath;
    std::vector<std::string> candidatePaths = {
        "assets/configs/client-rtype.json",        // map-editor config
        "../assets/configs/client-rtype.json",     // from build directory
        "../../assets/configs/client-rtype.json",  // two levels up
        "../assets/configs/client.json",           // from build directory
        "assets/configs/client.json",              // current directory
        "./assets/configs/client.json",            // explicit current
        "../../assets/configs/client.json",        // two levels up
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
    
    MapEditor editor(1000, 700);
    editor.loadAssets(registry);
    editor.run();
    
    return 0;
}
