#pragma once
#include <glm/glm.hpp>
#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    unsigned int VAO = 0, VBO = 0;
    int vertexCount  = 0;

    Mesh() = default;
    ~Mesh();
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& o) noexcept;
    Mesh& operator=(Mesh&& o) noexcept;

    // Wyslij vertices do GPU
    void upload();
    void draw() const;
    bool isUploaded() const { return VAO != 0; }
};