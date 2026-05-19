#pragma once
#include <glm/glm.hpp>
#include <array>
#include <utility>
#include <vector>
#include "scene/Wall.h"  // WallCutout

class InnerWall {
public:
    glm::vec2 startXZ;
    float     length;
    float     angle;
    float     thickness;
    float     height;

    std::array<glm::vec2, 4> cornersXZ;

    unsigned int VAO = 0, VBO = 0;
    int vertexCount  = 0;

    InnerWall() = default;
    InnerWall(glm::vec2 start, float len, float angleDeg, float thick, float h);
    InnerWall(const InnerWall&) = delete;
    InnerWall& operator=(const InnerWall&) = delete;
    InnerWall(InnerWall&& o) noexcept;
    InnerWall& operator=(InnerWall&& o) noexcept;
    ~InnerWall();

    void build();
    void buildWithCutouts(const std::vector<WallCutout>& cutouts);
    void draw() const;

    std::array<std::pair<glm::vec2,glm::vec2>, 4> getEdges() const;

private:
    void uploadVerts(const std::vector<float>& verts);
    void addFaceWithCutouts(std::vector<float>& verts,
                             glm::vec2 A, glm::vec2 B,
                             float wallLen,
                             glm::vec3 normal,
                             const std::vector<WallCutout>& cutouts);
    void addQuad(std::vector<float>& verts,
                 glm::vec2 A, glm::vec2 B,
                 float y0, float y1,
                 glm::vec3 normal);
};