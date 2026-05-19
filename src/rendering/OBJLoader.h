#pragma once
#include "Mesh.h"
#include <string>

class OBJLoader {
public:
    // Laduje plik .obj do Mesh.
    // Obsluguje: v, vt, vn, f (trojkaty i czworokaty).
    // Jezeli model nie ma UV — ustawia (0,0).
    // Jezeli model nie ma normalnych — generuje plaskaie.
    // Zwraca false przy bledzie.
    static bool load(const std::string& path, Mesh& outMesh);
};