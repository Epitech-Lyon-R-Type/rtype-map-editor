/*
** R-Type Map Editor
** Raylib-based map editor implementation
*/
#include "map_editor.hpp"
#include "map_serializer.hpp"
#include <algorithm>
#include <iostream>
#include <filesystem>
#include "tinyfiledialogs.h"

using namespace rtype::editor;

MapEditor::MapEditor(int width, int height) 
    : screenWidth(width), screenHeight(height) {
    InitWindow(screenWidth, screenHeight, "R-Type Map Editor");
    SetTargetFPS(60);
}

MapEditor::~MapEditor() {
    for (auto& [key, texture] : textures)
        UnloadTexture(texture);
    for (auto& [key, texture] : backgroundTextures)
        UnloadTexture(texture);
    if (backgroundTexture.id != 0)
        UnloadTexture(backgroundTexture);
    CloseWindow();
}

void MapEditor::loadAssets(const AssetRegistry& registry) {
    assetRegistry = registry;
    availableAssets.clear();
    availableBackgrounds.clear();
    
    // Scan for background files in assets/sprites/
    std::string bgFolder = "assets/sprites/";
    if (std::filesystem::exists(bgFolder)) {
        for (const auto& entry : std::filesystem::directory_iterator(bgFolder)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename.find("background") == 0) {
                    availableBackgrounds.push_back(filename);
                    // Preload background textures
                    Texture2D bgTex = LoadTexture(entry.path().string().c_str());
                    if (bgTex.id != 0) {
                        backgroundTextures[filename] = bgTex;
                        std::cout << "Loaded background: " << filename << std::endl;
                    }
                }
            }
        }
    }
    
    // List of placeable assets to display in palette
    std::vector<std::string> placeable = {
        "PLAYER", "SUOTRON", "ROCKER", "BIT_UNIT", "ZIPP", "MANX", "BOSS", "WALL", "POWER_UP"
    };
    
    std::vector<std::string> possiblePaths = {
        "../",                         // Relative to build directory
        "./",                          // Current directory
        "../../",                      // Two levels up
        ""                             // Direct path (if already complete)
    };
    
    // Load textures for ALL assets in registry (to have dimensions for all)
    for (const auto& [key, info] : registry) {
        std::cout << "Trying to load " << key << " from: " << info.spritePath << std::endl;
        
        bool loaded = false;
        for (const auto& basePath : possiblePaths) {
            std::string fullPath = basePath + info.spritePath;
            
            std::cout << "  Checking: " << fullPath << " ... ";
            if (std::filesystem::exists(fullPath)) {
                std::cout << "exists!" << std::endl;
                Texture2D tex = LoadTexture(fullPath.c_str());
                if (tex.id != 0) {
                    textures[key] = tex;
                    std::cout << "Loaded texture for " << key << " from " << fullPath << std::endl;
                    loaded = true;
                    break;
                } else {
                    std::cout << "LoadTexture failed" << std::endl;
                }
            } else {
                std::cout << "not found" << std::endl;
            }
        }
        
        if (!loaded) {
            std::cerr << "Texture file not found for " << key << ": " << info.spritePath << std::endl;
        }
    }
    
    for (const auto& assetName : placeable) {
        if (assetRegistry.find(assetName) != assetRegistry.end())
            availableAssets.push_back(assetName);
    }
    
    if (availableAssets.empty()) {
        availableAssets = {"PLAYER", "SUOTRON", "ROCKER", "WALL", "BIT_UNIT", "BOSS", "MANX", "POWER_UPS", "SHOOTS", "ZIPPER"};
    }
}

void MapEditor::handleInput() {
    //horizontal scroll with arrow keys
    float scrollSpeed = 10.0f;
    if (IsKeyDown(KEY_LEFT))
        scrollOffsetX += scrollSpeed;
    if (IsKeyDown(KEY_RIGHT))
        scrollOffsetX -= scrollSpeed;
    
    if (backgroundTexture.id != 0) {
        float totalMapWidth = (backgroundTexture.width * mapScale) * backgroundRepeatCount;
        float maxScrollX = totalMapWidth - canvasWidth;
        scrollOffsetX = std::max(-maxScrollX, std::min(scrollOffsetX, 0.0f));
    }
    
    //Save (Ctrl+S) and Load(Ctrl+O)
    bool ctrlDown = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    if (ctrlDown && IsKeyPressed(KEY_S)) {
        MapData map;
        map.width = static_cast<int>(canvasWidth);
        map.height = static_cast<int>(canvasHeight);
        map.entities = entities;
        map.assets = assetRegistry;
        if (!selectedBackground.empty()) {
            size_t underscorePos = selectedBackground.find('_');
            size_t dotPos = selectedBackground.find('.');
            if (underscorePos != std::string::npos && dotPos != std::string::npos && underscorePos < dotPos) {
                std::string numberStr = selectedBackground.substr(underscorePos + 1, dotPos - underscorePos - 1);
                try { map.id = std::stoi(numberStr); } catch (...) { map.id = 1; }
            }
        }

        std::filesystem::create_directories("maps");
        std::string serverPath = "maps/level_" + std::to_string(map.id) + "-server.json";
        std::string clientPath = "maps/level_" + std::to_string(map.id) + "-client.json";

        bool okServer = MapSerializer::saveServerLevel(serverPath, map);
        bool okClient = MapSerializer::saveClientLevel(clientPath, map);
        if (okServer) std::cout << "Saved server level to " << serverPath << std::endl;
        else std::cerr << "Failed to save server level to " << serverPath << std::endl;
        if (okClient) std::cout << "Saved client level to " << clientPath << std::endl;
        else std::cerr << "Failed to save client level to " << clientPath << std::endl;
    }
    if (ctrlDown && IsKeyPressed(KEY_O)) {
        // Open file dialog
        const char* filterPatterns[1] = {"*.json"};
        const char* openPath = tinyfd_openFileDialog(
            "Open Map",
            ".",
            1,
            filterPatterns,
            "JSON files",
            0
        );
        
        if (openPath) {
            MapData map = MapSerializer::loadMapFromFile(openPath);
            entities = map.entities;
            assetRegistry = map.assets.empty() ? assetRegistry : map.assets;
            nextId = 0;
            for (const auto& e : entities) nextId = std::max(nextId, e.id + 1);
            
            // Apply loaded background
            if (!map.backgroundName.empty()) {
                auto it = backgroundTextures.find(map.backgroundName);
                if (it != backgroundTextures.end()) {
                    backgroundTexture = it->second;
                    selectedBackground = map.backgroundName;
                }
            }
            backgroundRepeatCount = map.backgroundRepeatCount;
            
            std::cout << "Loaded map from " << openPath << " with " << entities.size() << " entities" << std::endl;
        }
    }

    //leftside palette: click to select asset for dragging
    float paletteX = 5;
    float paletteY = 40;
    float itemHeight = 50;
    
    for (size_t i = 0; i < availableAssets.size(); ++i) {
        Rectangle itemRect = {paletteX, paletteY + i * itemHeight, paletteWidth - 10, itemHeight - 5};
        if (CheckCollisionPointRec(GetMousePosition(), itemRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            draggingAsset = availableAssets[i];
            isDragging = true;
        }
    }
    
    //background selection
    float bgStartY = paletteY + availableAssets.size() * itemHeight + 20;
    for (size_t i = 0; i < availableBackgrounds.size(); ++i) {
        Rectangle bgRect = {paletteX, bgStartY + i * itemHeight, paletteWidth - 10, itemHeight - 5};
        if (CheckCollisionPointRec(GetMousePosition(), bgRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            selectedBackground = availableBackgrounds[i];
            auto it = backgroundTextures.find(selectedBackground);
            if (it != backgroundTextures.end()) {
                backgroundTexture = it->second;
                std::cout << "Selected background: " << selectedBackground << std::endl;
            }
        }
    }
    
    //mmap length control (repeat count)
    float repeatStartY = bgStartY + availableBackgrounds.size() * itemHeight + 20;
    Rectangle minusBtn = {paletteX, repeatStartY + 20, 30, 30};
    Rectangle plusBtn = {paletteX + 110, repeatStartY + 20, 30, 30};
    
    if (CheckCollisionPointRec(GetMousePosition(), minusBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        backgroundRepeatCount = std::max(1, backgroundRepeatCount - 1);
    }
    if (CheckCollisionPointRec(GetMousePosition(), plusBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        backgroundRepeatCount = std::min(10, backgroundRepeatCount + 1);
    }
    
    //drop on canvas
    if (isDragging && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePos = GetMousePosition();
        Rectangle canvasRect = {canvasX, canvasY, canvasWidth, canvasHeight};
        if (CheckCollisionPointRec(mousePos, canvasRect)) {
            EntityData e;
            e.id = nextId++;
            e.type = draggingAsset;
            e.x = (mousePos.x - canvasX - scrollOffsetX) / mapScale;
            e.y = (mousePos.y - canvasY) / mapScale;
            entities.push_back(e);
        }
        isDragging = false;
        draggingAsset.clear();
    }
    
    //click entity to select
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !isDragging) {
        Vector2 mousePos = GetMousePosition();
        if (CheckCollisionPointRec(mousePos, {canvasX, canvasY, canvasWidth, canvasHeight})) {
            selectedId = findEntityAt((mousePos.x - canvasX - scrollOffsetX) / mapScale, 
                                      (mousePos.y - canvasY) / mapScale);
        }
    }
    
    //drag selected entity
    if (selectedId != -1 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePos = GetMousePosition();
        auto it = std::find_if(entities.begin(), entities.end(), 
            [this](const EntityData& e) { return e.id == selectedId; });
        if (it != entities.end()) {
            it->x = (mousePos.x - canvasX - scrollOffsetX) / mapScale;
            it->y = (mousePos.y - canvasY) / mapScale;
        }
    }
    
    //delete selected entity
    if (selectedId != -1 && IsKeyPressed(KEY_DELETE)) {
        auto it = std::find_if(entities.begin(), entities.end(),
            [this](const EntityData& e) { return e.id == selectedId; });
        if (it != entities.end()) {
            entities.erase(it);
            selectedId = -1;
        }
    }
}

void MapEditor::draw() {
    BeginDrawing();
    ClearBackground(DARKGRAY);
    
    //title
    DrawText("R-Type Map Editor", canvasX + 10, 5, 20, WHITE);
    
    //left palette
    float paletteX = 5;
    float paletteY = 40;
    float itemHeight = 50;
    
    //palette background
    DrawRectangle(0, 0, paletteWidth, screenHeight, {40, 40, 40, 255});
    DrawRectangleLinesEx({0, 0, paletteWidth, screenHeight}, 2, WHITE);
    DrawText("Assets", paletteX + 5, 10, 14, WHITE);
    
    //draw assets in palette
    for (size_t i = 0; i < availableAssets.size(); ++i) {
        Rectangle itemRect = {paletteX, paletteY + i * itemHeight, paletteWidth - 10, itemHeight - 5};
        DrawRectangleRec(itemRect, DARKGRAY);
        DrawRectangleLinesEx(itemRect, 1, LIGHTGRAY);
        
        //draw texture preview in palette
        auto texIt = textures.find(availableAssets[i]);
        if (texIt != textures.end()) {
            auto regIt = assetRegistry.find(availableAssets[i]);
            if (regIt != assetRegistry.end()) {
                Rectangle sourceRec = {0, 0, (float)regIt->second.width, (float)regIt->second.height};
                Rectangle destRec = {paletteX + 5, paletteY + i * itemHeight + 5, 35, 35};
                
                DrawTexturePro(texIt->second, sourceRec, destRec, {0, 0}, 0.0f, WHITE);
            }
        }
        
        //draw asset name
        std::string assetName = availableAssets[i];
        if (assetName.length() > 6) {
            assetName = assetName.substr(0, 6) + ".";
        }
        DrawText(assetName.c_str(), paletteX + 45, paletteY + i * itemHeight + 17, 9, WHITE);
    }
    
    //backgrounds section
    float bgStartY = paletteY + availableAssets.size() * itemHeight + 20;
    DrawText("Backgrounds", paletteX + 5, bgStartY - 15, 12, WHITE);
    
    for (size_t i = 0; i < availableBackgrounds.size(); ++i) {
        Rectangle bgRect = {paletteX, bgStartY + i * itemHeight, paletteWidth - 10, itemHeight - 5};
        
        //highlight selected background
        if (availableBackgrounds[i] == selectedBackground) {
            DrawRectangleRec(bgRect, {100, 150, 255, 100});
        } else {
            DrawRectangleRec(bgRect, DARKGRAY);
        }
        DrawRectangleLinesEx(bgRect, 1, LIGHTGRAY);
        
        //draw background preview thumbnail
        auto bgIt = backgroundTextures.find(availableBackgrounds[i]);
        if (bgIt != backgroundTextures.end()) {
            Rectangle sourceRec = {0, 0, (float)bgIt->second.width, (float)bgIt->second.height};
            Rectangle destRec = {paletteX + 5, bgStartY + i * itemHeight + 5, 35, 35};
            DrawTexturePro(bgIt->second, sourceRec, destRec, {0, 0}, 0.0f, WHITE);
        }
        
        //draw background name
        std::string bgName = availableBackgrounds[i];
        if (bgName.length() > 10) {
            bgName = bgName.substr(0, 9) + ".";
        }
        DrawText(bgName.c_str(), paletteX + 42, bgStartY + i * itemHeight + 17, 8, WHITE);
    }
    
    // ===== map length control =====
    float repeatStartY = bgStartY + availableBackgrounds.size() * itemHeight + 20;
    DrawText("Map Length", paletteX + 5, repeatStartY, 12, WHITE);
    
    //minus button
    Rectangle minusBtn = {paletteX, repeatStartY + 20, 30, 30};
    DrawRectangleRec(minusBtn, DARKGRAY);
    DrawRectangleLinesEx(minusBtn, 1, LIGHTGRAY);
    DrawText("-", paletteX + 11, repeatStartY + 25, 20, WHITE);
    
    //display repeat count
    DrawText(TextFormat("x%d", backgroundRepeatCount), paletteX + 40, repeatStartY + 27, 14, WHITE);
    
    //plus button
    Rectangle plusBtn = {paletteX + 110, repeatStartY + 20, 30, 30};
    DrawRectangleRec(plusBtn, DARKGRAY);
    DrawRectangleLinesEx(plusBtn, 1, LIGHTGRAY);
    DrawText("+", paletteX + 120, repeatStartY + 25, 20, WHITE);
    
    //MAIN CANVAS
    //enable clipping to prevent drawing outside canvas area
    BeginScissorMode(canvasX, canvasY, canvasWidth, canvasHeight);
    
    if (backgroundTexture.id != 0) {
        float texWidth = static_cast<float>(backgroundTexture.width);
        float texHeight = static_cast<float>(backgroundTexture.height);
        mapScale = canvasHeight / texHeight;
        
        float scaledTexWidth = texWidth * mapScale;
        float scaledTexHeight = canvasHeight;
        
        Rectangle sourceRec = {0, 0, texWidth, texHeight};
        
        for (int i = 0; i < backgroundRepeatCount; ++i) {
            Rectangle destRec = {
                canvasX + scrollOffsetX + (i * scaledTexWidth), 
                canvasY, 
                scaledTexWidth, 
                scaledTexHeight
            };
            DrawTexturePro(backgroundTexture, sourceRec, destRec, {0, 0}, 0.0f, WHITE);
        }
    } else {
        DrawRectangle(canvasX, canvasY, canvasWidth, canvasHeight, BLACK);
    }
    
    //draw entities
    for (const auto& e : entities) {
        int size = 32;
        auto regIt = assetRegistry.find(e.type);
        if (regIt != assetRegistry.end()) {
            size = std::max(regIt->second.width, regIt->second.height);
        }
        
        float x = canvasX + e.x * mapScale + scrollOffsetX;
        float y = canvasY + e.y * mapScale;
        
        //draw texture if available, otherwise rectangle
        auto texIt = textures.find(e.type);
        if (texIt != textures.end()) {
            Texture2D& tex = texIt->second;
            Rectangle sourceRec = {0, 0, (float)regIt->second.width, (float)regIt->second.height};
            Rectangle destRec = {x - size/2.0f, y - size/2.0f, (float)size, (float)size};
            
            DrawTexturePro(tex, sourceRec, destRec, {0, 0}, 0.0f, WHITE);
            
            //highlight if selected in yellow
            if (e.id == selectedId)
                DrawRectangleLinesEx(destRec, 3, YELLOW);
        } else {
            // fallback to rectangle if no texture
            Rectangle rect = {x - size/2.0f, y - size/2.0f, (float)size, (float)size};
            
            if (e.id == selectedId) {
                DrawRectangleRec(rect, {255, 200, 0, 100});
                DrawRectangleLinesEx(rect, 2, YELLOW);
            } else {
                DrawRectangleRec(rect, {0, 150, 200, 100});
                DrawRectangleLinesEx(rect, 1, {100, 200, 255, 255});
            }
            
            DrawText(e.type.substr(0, 8).c_str(), x - size/2 + 2, y - size/2 + 2, 10, WHITE);
        }
    }
    
    //disable clipping
    EndScissorMode();
    
    //draw dragging preview (outside clipping to show on palette)
    if (isDragging) {
        Vector2 mousePos = GetMousePosition();
        auto texIt = textures.find(draggingAsset);
        
        if (texIt != textures.end()) {
            auto regIt = assetRegistry.find(draggingAsset);
            int size = 32;
            if (regIt != assetRegistry.end()) {
                size = std::max(regIt->second.width, regIt->second.height);
            }
            
            Rectangle sourceRec = {0, 0, (float)regIt->second.width, (float)regIt->second.height};
            Rectangle destRec = {mousePos.x - size/2.0f, mousePos.y - size/2.0f, (float)size, (float)size};
            
            DrawTexturePro(texIt->second, sourceRec, destRec, {0, 0}, 0.0f, Fade(WHITE, 0.6f));
        } else {
            DrawRectangle(mousePos.x - 16, mousePos.y - 16, 32, 32, {255, 100, 100, 150});
            DrawText(draggingAsset.substr(0, 8).c_str(), mousePos.x - 10, mousePos.y - 5, 10, WHITE);
        }
    }
    
    //Info bar at bottom
    DrawRectangle(0, screenHeight - 25, screenWidth, 25, DARKGRAY);
    DrawText(TextFormat("Entities: %zu | Selected: %s", entities.size(), 
        selectedId == -1 ? "None" : "Entity"), canvasX + 10, screenHeight - 20, 12, WHITE);
    DrawText("ARROWS scroll | DEL delete | Drag place | Ctrl+S save | Ctrl+O open", canvasX + 220, screenHeight - 20, 10, LIGHTGRAY);
    
    EndDrawing();
}

void MapEditor::update() {
    handleInput();
    draw();
}

void MapEditor::run() {
    while (!WindowShouldClose()) {
        update();
    }
}

int MapEditor::findEntityAt(float x, float y) const {
    for (auto it = entities.rbegin(); it != entities.rend(); ++it) {
        int size = 32;
        auto regIt = assetRegistry.find(it->type);
        if (regIt != assetRegistry.end()) {
            size = std::max(regIt->second.width, regIt->second.height);
        }
        
        float ex = it->x - size/2;
        float ey = it->y - size/2;
        if (x >= ex && x < ex + size && y >= ey && y < ey + size) {
            return it->id;
        }
    }
    return -1;
}
