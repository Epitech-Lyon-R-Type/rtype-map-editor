/*
** R-Type Map Editor
** Map serializer implementation
*/
#include "map_serializer.hpp"
#include "json.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

using json = nlohmann::json;

namespace rtype::editor {

std::string MapSerializer::mapToJson(const MapData& map) {
    const std::string gameConfig = map.gameConfigPath.empty()
        ? std::string("./assets/configs/rtype.json")
        : map.gameConfigPath;

    json j;
    j["game"] = gameConfig;
    j["map"] = {
        {"id", map.id},
        {"scrollSpeed", map.scrollSpeed},
        {"width", map.width},
        {"height", map.height},
        {"backgroundName", map.backgroundName},
        {"backgroundRepeatCount", map.backgroundRepeatCount}
    };
    j["waves"] = json::array();

    const auto typeRefs = loadTypeRefs(gameConfig);

    for (const auto& e : map.entities) {
        json wave;
        wave["x"] = e.x;
        wave["y"] = e.y;
        wave["name"] = e.type; // Keep explicit name to disambiguate duplicate refs
        auto it = typeRefs.find(e.type);
        if (it != typeRefs.end()) {
            wave["ref"] = it->second;
        } else {
            wave["ref"] = e.id; // Fallback to something stable if type is unknown
            std::cerr << "Unknown entity type '" << e.type << "', using id as ref" << std::endl;
        }
        j["waves"].push_back(wave);
    }
    
    return j.dump(4);
}

MapData MapSerializer::jsonToMap(const std::string& jsonStr) {
    MapData map;
    try {
        json j = json::parse(jsonStr);

        map.gameConfigPath = j.value("game", map.gameConfigPath);

        if (j.contains("map") && j["map"].is_object()) {
            const auto& meta = j["map"];
            map.id = meta.value("id", map.id);
            map.scrollSpeed = meta.value("scrollSpeed", map.scrollSpeed);
            map.width = meta.value("width", map.width);
            map.height = meta.value("height", map.height);
            map.backgroundName = meta.value("backgroundName", map.backgroundName);
            map.backgroundRepeatCount = meta.value("backgroundRepeatCount", map.backgroundRepeatCount);
        } else {
            map.width = j.value("width", map.width);
            map.height = j.value("height", map.height);
        }

        const auto typeRefs = loadTypeRefs(map.gameConfigPath);
        const auto refToType = invertTypeRefs(typeRefs);

        if (j.contains("waves") && j["waves"].is_array()) {
            int nextId = 0;
            for (const auto& w : j["waves"]) {
                EntityData entity;
                entity.id = nextId++;
                entity.x = w.value("x", 0.0F);
                entity.y = w.value("y", 0.0F);

                // Prefer explicit name if present (editor-added for disambiguation)
                if (w.contains("name") && w["name"].is_string()) {
                    entity.type = w["name"].get<std::string>();
                } else {
                    int ref = w.value("ref", -1);
                    auto it = refToType.find(ref);
                    entity.type = it != refToType.end() ? it->second : "UNKNOWN_" + std::to_string(ref);
                }

                map.entities.push_back(entity);
            }
        } else if (j.contains("entities") && j["entities"].is_array()) {
            for (const auto& e : j["entities"]) {
                EntityData entity;
                entity.id = e.value("id", 0);
                entity.type = e.value("type", "");
                entity.x = e.value("x", 0.0F);
                entity.y = e.value("y", 0.0F);
                map.entities.push_back(entity);
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error parsing JSON: " << ex.what() << std::endl;
    }
    
    return map;
}

bool MapSerializer::saveMapToFile(const std::string& filename, const MapData& map) {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) return false;
        file << mapToJson(map);
        file.close();
        return true;
    } catch (const std::exception& ex) {
        std::cerr << "Error saving map: " << ex.what() << std::endl;
        return false;
    }
}

MapData MapSerializer::loadMapFromFile(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) return MapData();
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        return jsonToMap(content);
    } catch (const std::exception& ex) {
        std::cerr << "Error loading map: " << ex.what() << std::endl;
        return MapData();
    }
}

std::unordered_map<std::string, int> MapSerializer::loadTypeRefs(const std::string& gameConfigPath) {
    std::unordered_map<std::string, int> refs;
    try {
        std::ifstream file(gameConfigPath);
        if (!file.is_open()) {
            std::cerr << "Cannot open game config: " << gameConfigPath << std::endl;
            return refs;
        }

        json j;
        file >> j;

        if (j.contains("entities") && j["entities"].is_object()) {
            for (const auto& [name, entity] : j["entities"].items()) {
                if (entity.contains("type") && entity["type"].is_object()) {
                    int ref = entity["type"].value("ref", -1);
                    if (ref != -1) {
                        refs[name] = ref;
                    }
                }
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error reading type refs: " << ex.what() << std::endl;
    }
    return refs;
}

std::unordered_map<int, std::string> MapSerializer::invertTypeRefs(const std::unordered_map<std::string, int>& refs) {
    std::unordered_map<int, std::string> inverted;

    std::vector<std::pair<std::string, int>> ordered(refs.begin(), refs.end());
    std::sort(ordered.begin(), ordered.end(), [](const auto& a, const auto& b) {
        return a.first < b.first; // alphabetical order on the entity name
    });

    for (const auto& [name, ref] : ordered) {
        auto it = inverted.find(ref);
        if (it != inverted.end()) {
            std::cerr << "Duplicate ref " << ref << ": keeping '" << name << "' over '" << it->second << "'" << std::endl;
        }
        inverted[ref] = name;
    }

    return inverted;
}

bool MapSerializer::saveServerLevel(const std::string& filename, const MapData& map) {
    try {
        const std::string serverGameConfig = "config/game/rtype.json";
        const auto typeRefs = loadTypeRefs(serverGameConfig);

        nlohmann::json j;
        j["game"] = serverGameConfig;
        j["systems"] = nlohmann::json::array({
            "ScrollSystem",
            "WaveSystem",
            "AISystem",
            "MovementSystem",
            "HitboxSystem",
            "WeaponSystem",
            "CleanupSystem"
        });
        j["spawn_points"] = nlohmann::json::array();
        j["startup"] = nlohmann::json::array();

        j["level_data"] = nlohmann::json::array();
        for (const auto& e : map.entities) {
            int ref = -1;
            auto it = typeRefs.find(e.type);
            if (it != typeRefs.end()) ref = it->second;
            nlohmann::json entry;
            entry["ref"] = ref;
            entry["position"] = { {"x", e.x}, {"y", e.y} };
            j["level_data"].push_back(entry);
        }

        std::ofstream file(filename);
        if (!file.is_open()) return false;
        file << j.dump(4);
        file.close();
        return true;
    } catch (const std::exception& ex) {
        std::cerr << "Error saving server level: " << ex.what() << std::endl;
        return false;
    }
}

// New: Save in client level format (minimal – no positions)
bool MapSerializer::saveClientLevel(const std::string& filename, const MapData& map) {
    try {
        const std::string clientGameConfig = "config/game/client-rtype.json";
        nlohmann::json j;
        j["game"] = clientGameConfig;
        j["systems"] = nlohmann::json::array({
            "GameInteractionSystem",
            "ScrollSystem",
            "MovementSystem",
            "HitboxSystem",
            "ClearScreenSystem",
            "DrawingStartSystem",
            "BackgroundRenderingSystem",
            "CameraStartSystem",
            "HitboxRenderingSystem",
            "RectRenderingSystem",
            "SpriteRenderingSystem",
            "TextRenderingSystem",
            "CameraEndSystem",
            "DrawingEndSystem"
        });
        j["sprites"] = nlohmann::json::object();
        j["startup"] = nlohmann::json::array();

        std::ofstream file(filename);
        if (!file.is_open()) return false;
        file << j.dump(4);
        file.close();
        return true;
    } catch (const std::exception& ex) {
        std::cerr << "Error saving client level: " << ex.what() << std::endl;
        return false;
    }
}

}
