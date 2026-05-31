#include "Furniture.h"
#include "rendering/Mesh.h"
#include "rendering/OBJLoader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

bool FurnitureManager::addItem(const std::string& modelFile,
                                const std::string& textureFile,
                                glm::vec3 pos, float rotY, float scl)
{
    FurnitureItem item;
    item.modelFile    = modelFile;
    item.textureFile  = textureFile;
    item.position     = pos;
    item.rotationY    = rotY;
    item.scale        = scl;
    item.isProcedural = false;

    auto meshIt = meshCache.find(modelFile);
    if (meshIt != meshCache.end()) {
        item.mesh = meshIt->second;
    } else {
        auto m = std::make_shared<Mesh>();
        std::string path = "assets/models/" + modelFile;
        if (!OBJLoader::load(path, *m)) {
            std::cerr << "[Furniture] Blad ladowania: " << path << "\n";
            return false;
        }
        meshCache[modelFile] = m;
        item.mesh = m;
    }

    if (!textureFile.empty()) {
        auto texIt = texCache.find(textureFile);
        if (texIt != texCache.end()) {
            item.texture = texIt->second;
        } else {
            auto t = std::make_shared<Texture>();
            std::string tpath = "assets/textures/" + textureFile;
            if (t->load(tpath)) {
                texCache[textureFile] = t;
                item.texture = t;
            } else {
                std::cerr << "[Furniture] Brak tekstury: " << tpath << "\n";
                item.texture = nullptr;
            }
        }
    }

    items.push_back(std::move(item));
    return true;
}

void FurnitureManager::addBuiltinItem(FurnitureType type,
                                       glm::vec3 pos, float rotY, float scl)
{
    FurnitureItem item;
    item.isProcedural    = true;
    item.position        = pos;
    item.rotationY       = rotY;
    item.scale           = scl;
    item.modelFile       = furnitureName(type);
    item.proceduralParts = FurnitureLibrary::buildParts(type);
    items.push_back(std::move(item));
}

void FurnitureManager::draw(Shader& shader) const
{
    for (const auto& item : items) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, item.position);
        model = glm::rotate(model, glm::radians(item.rotationY), glm::vec3(0,1,0));
        model = glm::scale(model, glm::vec3(item.scale));
        shader.setMat4("model", model);

        if (item.isProcedural) {
            shader.setInt("useTexture", 0);
            for (const auto& part : item.proceduralParts) {
                if (!part.mesh || !part.mesh->isUploaded()) continue;
                shader.setVec3("objectColor", part.color);
                part.mesh->draw();
            }
        } else {
            if (!item.mesh || !item.mesh->isUploaded()) continue;
            if (item.texture && item.texture->isLoaded()) {
                item.texture->bind(0);
                shader.setInt("diffuseTexture", 0);
                shader.setInt("useTexture", 1);
            } else {
                shader.setVec3("objectColor", glm::vec3(0.7f,0.55f,0.35f));
                shader.setInt("useTexture", 0);
            }
            item.mesh->draw();
        }
    }
}

void FurnitureManager::clear() {
    items.clear();
}