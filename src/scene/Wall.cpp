#include "Wall.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <algorithm>

Wall::Wall(glm::vec2 s, glm::vec2 e, float h) : start(s), end(e), height(h) {}

Wall::Wall(Wall&& o) noexcept
    : start(o.start), end(o.end), height(o.height)
    , VAO(o.VAO), VBO(o.VBO), vertexCount(o.vertexCount)
{ o.VAO=0; o.VBO=0; }

Wall& Wall::operator=(Wall&& o) noexcept {
    if (this!=&o) {
        if(VAO) glDeleteVertexArrays(1,&VAO);
        if(VBO) glDeleteBuffers(1,&VBO);
        start=o.start; end=o.end; height=o.height;
        VAO=o.VAO; VBO=o.VBO; vertexCount=o.vertexCount;
        o.VAO=0; o.VBO=0;
    }
    return *this;
}

Wall::~Wall() {
    if(VAO) glDeleteVertexArrays(1,&VAO);
    if(VBO) glDeleteBuffers(1,&VBO);
}

glm::vec3 Wall::computeNormal(glm::vec2 centroid2D) const
{
    glm::vec2 d2  = end - start;
    float     len = glm::length(d2);
    if(len < 0.001f) return {0,0,1};
    glm::vec2 dir = d2 / len;
    glm::vec2 n1  = {-dir.y,  dir.x};
    glm::vec2 n2  = { dir.y, -dir.x};
    glm::vec2 mid = (start+end)*0.5f;
    glm::vec2 nXZ = (glm::dot(n1, centroid2D-mid) > 0) ? n1 : n2;
    return {nXZ.x, 0.0f, nXZ.y};
}

void Wall::addQuad(std::vector<float>& verts,
                   glm::vec2 A, glm::vec2 B,
                   float y0, float y1,
                   float u0, float u1,
                   glm::vec3 n)
{
    // Format: pos(3) + normal(3) + uv(2) = 8 floatow na wierzcholek
    // 2 trojkaty = 6 wierzcholkow
    auto add = [&](glm::vec2 p, float y, float u, float v){
        verts.insert(verts.end(),
            {p.x, y, p.y, n.x, n.y, n.z, u, v});
    };
    add(A,y0,u0,0); add(B,y0,u1,0); add(B,y1,u1,1);
    add(A,y0,u0,0); add(B,y1,u1,1); add(A,y1,u0,1);
}

void Wall::uploadVerts(const std::vector<float>& verts)
{
    if(VAO) glDeleteVertexArrays(1,&VAO);
    if(VBO) glDeleteBuffers(1,&VBO);

    vertexCount = (int)(verts.size() / 8);
    if(vertexCount == 0) return;

    glGenVertexArrays(1,&VAO); glGenBuffers(1,&VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,
        verts.size()*sizeof(float),verts.data(),GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),
        (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,8*sizeof(float),
        (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void Wall::build(glm::vec2 centroid2D)
{
    buildWithCutouts(centroid2D, {});
}

void Wall::buildWithCutouts(glm::vec2 centroid2D,
                             const std::vector<WallCutout>& cutouts)
{
    glm::vec2 d2  = end - start;
    float     len = glm::length(d2);
    if(len < 0.001f) return;

    glm::vec3 normal = computeNormal(centroid2D);
    glm::vec2 dir    = d2 / len;

    // Posortuj wyciecia wg offsetStart
    std::vector<WallCutout> sorted = cutouts;
    std::sort(sorted.begin(), sorted.end(),
        [](const WallCutout& a, const WallCutout& b){
            return a.offsetStart < b.offsetStart;
        });

    std::vector<float> verts;
    float cur = 0.0f;  // aktualny offset wzdluz sciany

    for (const auto& c : sorted) {
        float cs = glm::clamp(c.offsetStart, 0.0f, len);
        float ce = glm::clamp(c.offsetEnd,   0.0f, len);
        float cb = glm::clamp(c.yBottom,     0.0f, height);
        float ct = glm::clamp(c.yTop,        0.0f, height);
        if(ce <= cs) continue;

        // Sekcja sciany przed wycieciem
        if (cur < cs) {
            glm::vec2 A = start + dir*cur;
            glm::vec2 B = start + dir*cs;
            addQuad(verts, A, B, 0.0f, height, cur, cs, normal);
        }

        // Dolna czesc wyciecia (parapet okna lub zero dla drzwi)
        if (cb > 0.001f) {
            glm::vec2 A = start + dir*cs;
            glm::vec2 B = start + dir*ce;
            addQuad(verts, A, B, 0.0f, cb, cs, ce, normal);
        }

        // Gorna czesc wyciecia (nadprozе)
        if (ct < height - 0.001f) {
            glm::vec2 A = start + dir*cs;
            glm::vec2 B = start + dir*ce;
            addQuad(verts, A, B, ct, height, cs, ce, normal);
        }

        cur = ce;
    }

    // Pozostala czesc sciany po ostatnim wyjeciu
    if (cur < len - 0.001f) {
        glm::vec2 A = start + dir*cur;
        glm::vec2 B = start + dir*len;
        addQuad(verts, A, B, 0.0f, height, cur, len, normal);
    }

    uploadVerts(verts);
}

void Wall::draw() const {
    if(!VAO || vertexCount==0) return;
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}