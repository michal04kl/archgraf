#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include "rendering/Mesh.h"
#include "rendering/Texture.h"
#include "core/Shader.h"

struct FurnitureItem {
    std::string modelFile;    // np. "chair.obj"
    std::string textureFile;  // np. "chair.png" lub "" jesli brak

    glm::vec3 position  = {0, 0, 0};
    float     rotationY = 0.0f;   // stopnie
    float     scale     = 1.0f;

    std::shared_ptr<Mesh>    mesh;
    std::shared_ptr<Texture> texture;  // moze byc nullptr
};

class FurnitureManager {
public:
    std::vector<FurnitureItem> items;

    // Keszowane zasoby (model -> mesh/texture)
    std::map<std::string, std::shared_ptr<Mesh>>    meshCache;
    std::map<std::string, std::shared_ptr<Texture>> texCache;

    // Dodaje mebel. Laduje model jesli jeszcze nie w keszu.
    // textureFile moze byc "" — wtedy mebel renderuje sie kolorem.
    bool addItem(const std::string& modelFile,
                 const std::string& textureFile,
                 glm::vec3 pos, float rotY, float scl);

    void draw(Shader& shader) const;

    void clear();
};