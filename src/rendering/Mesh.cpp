#include "Mesh.h"
#include <glad/glad.h>

Mesh::~Mesh() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
}

Mesh::Mesh(Mesh&& o) noexcept
    : vertices(std::move(o.vertices))
    , VAO(o.VAO), VBO(o.VBO), vertexCount(o.vertexCount)
{ o.VAO = 0; o.VBO = 0; }

Mesh& Mesh::operator=(Mesh&& o) noexcept {
    if (this != &o) {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        vertices = std::move(o.vertices);
        VAO=o.VAO; VBO=o.VBO; vertexCount=o.vertexCount;
        o.VAO=0; o.VBO=0;
    }
    return *this;
}

void Mesh::upload() {
    if (vertices.empty()) return;
    if (VAO) { glDeleteVertexArrays(1,&VAO); glDeleteBuffers(1,&VBO); }

    vertexCount = (int)vertices.size();
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
        vertexCount * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // position (0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    // normal (1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    // texCoord (2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Mesh::draw() const {
    if (!VAO) return;
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}