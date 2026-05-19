#include "OBJLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <algorithm>

static glm::vec3 computeFlatNormal(const glm::vec3& a,
                                   const glm::vec3& b,
                                   const glm::vec3& c)
{
    return glm::normalize(glm::cross(b - a, c - a));
}

bool OBJLoader::load(const std::string& path, Mesh& outMesh)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[OBJ] Nie mozna otworzyc: " << path << "\n";
        return false;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;

    // Tymczasowe indeksy
    struct FaceVert { int pos=-1, tex=-1, nor=-1; };

    std::vector<std::vector<FaceVert>> faces;

    std::string line;
    while (std::getline(file, line)) {
        // Usun znaki powrotu karetki
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (line.empty() || line[0] == '#') continue;

        std::istringstream ss(line);
        std::string token;
        ss >> token;

        if (token == "v") {
            glm::vec3 p;
            ss >> p.x >> p.y >> p.z;
            positions.push_back(p);
        }
        else if (token == "vt") {
            glm::vec2 t;
            ss >> t.x >> t.y;
            texCoords.push_back(t);
        }
        else if (token == "vn") {
            glm::vec3 n;
            ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (token == "f") {
            // Wczytaj dowolna liczbe wierzcholkow sciany
            std::vector<FaceVert> fv;
            std::string vstr;
            while (ss >> vstr) {
                FaceVert fvert;
                // Formaty: v  lub  v/t  lub  v/t/n  lub  v//n
                std::replace(vstr.begin(), vstr.end(), '/', ' ');
                std::istringstream vs(vstr);
                std::string idx;
                int k = 0;
                while (vs >> idx) {
                    int val = std::stoi(idx);
                    if (k == 0) fvert.pos = val > 0 ? val-1 : (int)positions.size()+val;
                    if (k == 1) fvert.tex = val > 0 ? val-1 : (int)texCoords.size()+val;
                    if (k == 2) fvert.nor = val > 0 ? val-1 : (int)normals.size()+val;
                    ++k;
                }
                fv.push_back(fvert);
            }
            if (fv.size() >= 3) faces.push_back(fv);
        }
    }

    outMesh.vertices.clear();

    for (auto& fv : faces) {
        // Triangulacja fan: (0,1,2), (0,2,3), (0,3,4)...
        for (int i = 1; i + 1 < (int)fv.size(); ++i) {
            int tri[3] = {0, i, i+1};
            // Pobierz 3 pozycje do obliczenia normalnej plaskiej
            glm::vec3 p[3];
            for (int t = 0; t < 3; ++t)
                p[t] = (fv[tri[t]].pos >= 0 && fv[tri[t]].pos < (int)positions.size())
                       ? positions[fv[tri[t]].pos] : glm::vec3(0);
            glm::vec3 flatN = computeFlatNormal(p[0], p[1], p[2]);

            for (int t = 0; t < 3; ++t) {
                FaceVert& fvert = fv[tri[t]];
                Vertex v;
                v.position = (fvert.pos >= 0 && fvert.pos < (int)positions.size())
                             ? positions[fvert.pos] : glm::vec3(0);
                v.normal   = (fvert.nor >= 0 && fvert.nor < (int)normals.size())
                             ? normals[fvert.nor] : flatN;
                v.texCoord = (fvert.tex >= 0 && fvert.tex < (int)texCoords.size())
                             ? texCoords[fvert.tex] : glm::vec2(0);
                outMesh.vertices.push_back(v);
            }
        }
    }

    outMesh.upload();
    std::cout << "[OBJ] Zaladowano: " << path
              << " (" << outMesh.vertexCount << " wierzcholkow)\n";
    return true;
}