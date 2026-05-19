#pragma once

#include <string>
#include <glm/glm.hpp>

// Klasa opakowujaca program shaderow OpenGL.
// Laduje pliki GLSL, kompiluje i linkuje.
class Shader {
public:
    unsigned int ID;  // ID programu shaderow

    // Konstruktor: sciezki do plikow vertex i fragment shadera
    Shader(const char* vertexPath, const char* fragmentPath);

    // Aktywacja programu shaderow
    void use() const;

    // Settery uniformow
    void setBool  (const std::string& name, bool value)         const;
    void setInt   (const std::string& name, int value)          const;
    void setFloat (const std::string& name, float value)        const;
    void setVec2  (const std::string& name, const glm::vec2& v) const;
    void setVec3  (const std::string& name, const glm::vec3& v) const;
    void setVec4  (const std::string& name, const glm::vec4& v) const;
    void setMat4  (const std::string& name, const glm::mat4& m) const;
    void setInt (const std::string& name, int value)   const;
    void setBool(const std::string& name, bool value)  const;

private:
    // Sprawdza bledy kompilacji/linkowania shaderow
    void checkCompileErrors(unsigned int shader, const std::string& type);
};