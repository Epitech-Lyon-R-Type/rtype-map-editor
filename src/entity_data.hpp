/*
** R-Type Map Editor
** Entity data structure
*/
#pragma once
#include <string>

namespace rtype::editor {
struct EntityData {
    int id = 0;
    std::string type;
    float x = 0.0F;
    float y = 0.0F;
    EntityData() = default;
    EntityData(int id, const std::string& type, float x, float y)
        : id(id), type(type), x(x), y(y) {}
};
}
