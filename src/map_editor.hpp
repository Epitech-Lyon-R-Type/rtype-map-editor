/*
** R-Type Map Editor
** Raylib-based map editor
*/
#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <raylib.h>
#include "entity_data.hpp"
#include "asset_info.hpp"

namespace rtype::editor {

class MapEditor {
public:
    MapEditor(int screenWidth, int screenHeight);
    ~MapEditor();
    
    void run();
    void loadAssets(const AssetRegistry& registry);
    std::vector<EntityData> getEntities() const { return entities; }
    
private:
    int screenWidth;
    int screenHeight;
    std::vector<EntityData> entities;
    std::vector<std::string> availableAssets;
    std::vector<std::string> availableBackgrounds;  // List of background files
    AssetRegistry assetRegistry;
    std::unordered_map<std::string, Texture2D> textures;
    std::unordered_map<std::string, Texture2D> backgroundTextures;  // Background textures
    std::string selectedBackground = "";  // Currently selected background
    Texture2D backgroundTexture = {0};  // Active background texture for canvas
    
    int nextId = 0;
    int selectedId = -1;
    std::string draggingAsset;
    bool isDragging = false;
    
    // UI state - Palette on left, canvas on right
    int gridSize = 32;
    float paletteWidth = 150;
    float canvasX = 160;
    float canvasY = 40;
    float canvasWidth = 800;
    float canvasHeight = 600;
    
    // Scroll and zoom state
    float scrollOffsetX = 0.0f;  // Horizontal scroll offset
    float mapScale = 1.0f;       // Scale factor based on canvas height
    int backgroundRepeatCount = 1;  // Number of times to repeat background horizontally
    
    // Helper methods
    void update();
    void draw();
    void handleInput();
    int findEntityAt(float x, float y) const;
};

} // namespace rtype::editor
