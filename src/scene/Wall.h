#pragma once
#include <glm/glm.hpp>
#include <vector>

// Wycięcie w ścianie (drzwi lub okno)
struct WallCutout {
    float offsetStart; // przesuniecie od poczatku sciany (m)
    float offsetEnd;   // offsetStart + szerokosc
    float yBottom;     // dolna krawedz wyciecia (0 dla drzwi)
    float yTop;        // gorna krawedz wyciecia (wysokosc drzwi lub parapet+wysokosc)
};

class Wall {
public:
    glm::vec2 start, end;
    float height = 2.7f;
    unsigned int VAO = 0, VBO = 0;
    int vertexCount = 0;

    Wall() = default;
    Wall(glm::vec2 s, glm::vec2 e, float h);
    Wall(const Wall&) = delete;
    Wall& operator=(const Wall&) = delete;
    Wall(Wall&& o) noexcept;
    Wall& operator=(Wall&& o) noexcept;
    ~Wall();

    // Buduje geometrie bez wyciec
    void build(glm::vec2 centroid2D);

    // Buduje geometrie z wycieciami (dla drzwi i okien)
    void buildWithCutouts(glm::vec2 centroid2D,
                          const std::vector<WallCutout>& cutouts);

    void draw() const;

private:
    // Dodaje quad do bufora wierzcholkow
    void addQuad(std::vector<float>& verts,
                 glm::vec2 A, glm::vec2 B,   // punkty XZ
                 float y0, float y1,          // dolna i gorna krawedz
                 float u0, float u1,          // wspolrzedne UV (u)
                 glm::vec3 normal);

    void uploadVerts(const std::vector<float>& verts);
    glm::vec3 computeNormal(glm::vec2 centroid2D) const;
};