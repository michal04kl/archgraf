#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include "rendering/Mesh.h"
#include "rendering/Texture.h"
#include "core/Shader.h"
#include "FurnitureLibrary.h"

struct FurnitureItem {
    std::string modelFile;
    std::string textureFile;

    glm::vec3 position  = {0, 0, 0};
    float     rotationY = 0.0f;
    float     scale     = 1.0f;

    // OBJ-based
    std::shared_ptr<Mesh>    mesh;
    std::shared_ptr<Texture> texture;

    // Wbudowany (procedural)
    bool isProcedural = false;
    std::vector<FurniturePart> proceduralParts;
};

class FurnitureManager {
public:
    std::vector<FurnitureItem> items;
    std::map<std::string, std::shared_ptr<Mesh>>    meshCache;
    std::map<std::string, std::shared_ptr<Texture>> texCache;

    bool addItem(const std::string& modelFile,
                 const std::string& textureFile,
                 glm::vec3 pos, float rotY, float scl);

    void addBuiltinItem(FurnitureType type,
                        glm::vec3 pos, float rotY, float scl);

    void draw(Shader& shader) const;
    void clear();
};