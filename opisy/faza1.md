# PROJEKT: Aplikacja do projektowania wnętrz 3D — Dokumentacja Kontekstowa

> Ten plik zawiera WSZYSTKO potrzebne do kontynuowania projektu w nowym czacie.
> Wklej go i napisz: **"Jedziemy z fazą X"** — i od razu lecimy z kodem.

---

## ŚRODOWISKO — GOTOWE I DZIAŁAJĄCE ✅

### System
- **OS:** Windows 10/11
- **IDE:** Visual Studio Code

### Kompilator
- **MSYS2 → g++ 14.2.0** (ucrt64)
- Ścieżka: `C:/msys64/ucrt64/bin/`
- Kompilator działa bez dodatkowej instalacji (MSYS2 już zainstalowany)

### System budowania
- **CMake** (MinGW Makefiles)
- Build przez: `cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug` + `mingw32-make`

### Biblioteki — ZAINSTALOWANE i DZIAŁAJĄCE
| Biblioteka | Wersja | Status |
|---|---|---|
| GLAD | Core 3.3 | ✅ działa |
| GLFW | najnowsza, Windows 64-bit pre-compiled | ✅ działa |
| SDL | **SDL3** wersja 3.4.2 (mingw) | ✅ działa |
| OpenGL | 3.3 Core Profile | ✅ działa |
| GLM | 1.0.1 (pełna, light release) | ✅ działa |

### WAŻNE — SDL3 różnice od SDL2
- `SDL_Init()` zwraca `bool`, nie `int` → używamy `if (!SDL_Init(...))`
- Brak `libSDL3main.a` — SDL3 obsługuje main inaczej
- Header: `#include <SDL3/SDL.h>` (nie `<SDL2/SDL.h>`)
- DLL: `SDL3.dll` (nie `SDL2.dll`)

---

## STRUKTURA PROJEKTU NA DYSKU (aktualna po Fazie 1)

```
C:\Users\studia\Desktop\proj_graf\
├── CMakeLists.txt
├── PROJEKT_OpenGL_Mieszkanie.md
├── shaders/                            ← shadery w GŁÓWNYM folderze (nie w src!)
│   ├── phong.vert
│   ├── phong.frag
│   ├── flat.vert
│   └── flat.frag
├── .vscode/
│   ├── c_cpp_properties.json           ← dodany w Fazie 1 (IntelliSense)
│   ├── launch.json
│   └── settings.json
├── build/
│   ├── OpenGLProject.exe
│   ├── SDL3.dll
│   └── shaders/                        ← kopiowane automatycznie przez CMake
│       ├── phong.vert
│       ├── phong.frag
│       ├── flat.vert
│       └── flat.frag
├── src/
│   ├── main.cpp
│   └── core/
│       ├── Shader.h
│       ├── Shader.cpp
│       ├── Camera.h
│       └── Camera.cpp
└── external/
    ├── glad/
    │   ├── include/glad/glad.h
    │   ├── include/KHR/khrplatform.h
    │   └── src/glad.c
    ├── glfw/
    │   ├── include/GLFW/glfw3.h
    │   ├── include/GLFW/glfw3native.h
    │   └── lib-mingw-w64/
    │       ├── glfw3.dll
    │       ├── libglfw3.a
    │       └── libglfw3dll.a
    ├── glm/                            ← dodane w Fazie 1 (PEŁNA biblioteka!)
    │   └── glm/
    │       ├── glm.hpp
    │       ├── detail/                 ← wymagane! (m.in. _fixes.hpp)
    │       ├── gtc/                    ← wymagane! (matrix_transform, type_ptr)
    │       └── ... (setki innych plików)
    └── SDL2/                           ← folder SDL2 zawiera SDL3!
        ├── include/SDL3/SDL.h (+ reszta)
        └── lib/x64/
            ├── libSDL3.dll.a
            └── SDL3.dll
```

---

## AKTUALNE PLIKI KONFIGURACYJNE

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.16)
project(OpenGLProject VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/core/Shader.cpp
    src/core/Camera.cpp
    external/glad/src/glad.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    external/glad/include
    external/glfw/include
    external/SDL2/include
    external/glm
    src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/external/glfw/lib-mingw-w64/libglfw3.a
    ${CMAKE_SOURCE_DIR}/external/SDL2/lib/x64/libSDL3.dll.a
    opengl32
    gdi32
    winmm
    imm32
    ole32
    oleaut32
    version
    uuid
    advapi32
    setupapi
    shell32
)

# Kopiuj SDL3.dll obok exe
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/external/SDL2/lib/x64/SDL3.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

# Kopiuj folder shaders/ obok exe
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/shaders"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
)
```

### .vscode/settings.json
```json
{
    "cmake.configureArgs": ["-DCMAKE_BUILD_TYPE=Debug"],
    "cmake.generator": "MinGW Makefiles",
    "cmake.buildDirectory": "${workspaceFolder}/build",
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
}
```

### .vscode/launch.json
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug OpenGL",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/OpenGLProject.exe",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}/build",
            "environment": [],
            "externalConsole": true,
            "MIMode": "gdb",
            "miDebuggerPath": "C:/msys64/mingw64/bin/gdb.exe",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        }
    ]
}
```

### .vscode/c_cpp_properties.json ← NOWY (dodany w Fazie 1)
```json
{
    "configurations": [
        {
            "name": "Windows",
            "includePath": [
                "${workspaceFolder}/src",
                "${workspaceFolder}/external/glad/include",
                "${workspaceFolder}/external/glfw/include",
                "${workspaceFolder}/external/SDL2/include",
                "${workspaceFolder}/external/glm"
            ],
            "defines": [],
            "compilerPath": "C:/msys64/ucrt64/bin/g++.exe",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "windows-gcc-x64",
            "configurationProvider": "ms-vscode.cmake-tools"
        }
    ],
    "version": 4
}
```

---

## KOD ŹRÓDŁOWY — FAZA 1

### shaders/phong.vert
```glsl
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos     = vec3(model * vec4(aPos, 1.0));
    Normal      = mat3(transpose(inverse(model))) * aNormal;
    TexCoord    = aTexCoord;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
```

### shaders/phong.frag
```glsl
#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3  viewPos;
uniform vec3  objectColor;
uniform vec3  lightDirection;
uniform vec3  lightAmbient;
uniform vec3  lightDiffuse;
uniform vec3  lightSpecular;
uniform float shininess;

void main()
{
    // Ambient
    vec3 ambient = lightAmbient * objectColor;

    // Diffuse
    vec3  norm     = normalize(Normal);
    vec3  lightDir = normalize(lightDirection);
    float diff     = max(dot(norm, lightDir), 0.0);
    vec3  diffuse  = lightDiffuse * diff * objectColor;

    // Specular (Phong)
    vec3  viewDir    = normalize(viewPos - FragPos);
    vec3  reflectDir = reflect(-lightDir, norm);
    float spec       = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3  specular   = lightSpecular * spec;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
```

### shaders/flat.vert
```glsl
#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
```

### shaders/flat.frag
```glsl
#version 330 core

out vec4 FragColor;
uniform vec4 color;

void main()
{
    FragColor = color;
}
```

### src/core/Shader.h
```cpp
#pragma once

#include <string>
#include <glm/glm.hpp>

// Klasa opakowujaca program shaderow OpenGL.
// Laduje pliki GLSL, kompiluje i linkuje.
class Shader {
public:
    unsigned int ID;

    Shader(const char* vertexPath, const char* fragmentPath);

    void use() const;

    void setBool  (const std::string& name, bool value)         const;
    void setInt   (const std::string& name, int value)          const;
    void setFloat (const std::string& name, float value)        const;
    void setVec2  (const std::string& name, const glm::vec2& v) const;
    void setVec3  (const std::string& name, const glm::vec3& v) const;
    void setVec4  (const std::string& name, const glm::vec4& v) const;
    void setMat4  (const std::string& name, const glm::mat4& m) const;

private:
    void checkCompileErrors(unsigned int shader, const std::string& type);
};
```

### src/core/Shader.cpp
```cpp
#include "Shader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    std::string   vertexCode, fragmentCode;
    std::ifstream vShaderFile, fShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vss, fss;
        vss << vShaderFile.rdbuf();
        fss << fShaderFile.rdbuf();
        vertexCode   = vss.str();
        fragmentCode = fss.str();
    }
    catch (std::ifstream::failure& e) {
        std::cerr << "[Shader] Blad odczytu pliku: " << e.what() << "\n";
        std::cerr << "  vertex:   " << vertexPath   << "\n";
        std::cerr << "  fragment: " << fragmentPath << "\n";
    }

    const char* vCode = vertexCode.c_str();
    const char* fCode = fragmentCode.c_str();

    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vCode, nullptr);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fCode, nullptr);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::use() const { glUseProgram(ID); }

void Shader::setBool (const std::string& name, bool value)          const
{ glUniform1i (glGetUniformLocation(ID, name.c_str()), (int)value); }

void Shader::setInt  (const std::string& name, int value)           const
{ glUniform1i (glGetUniformLocation(ID, name.c_str()), value); }

void Shader::setFloat(const std::string& name, float value)         const
{ glUniform1f (glGetUniformLocation(ID, name.c_str()), value); }

void Shader::setVec2 (const std::string& name, const glm::vec2& v)  const
{ glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(v)); }

void Shader::setVec3 (const std::string& name, const glm::vec3& v)  const
{ glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(v)); }

void Shader::setVec4 (const std::string& name, const glm::vec4& v)  const
{ glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(v)); }

void Shader::setMat4 (const std::string& name, const glm::mat4& m)  const
{ glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(m)); }

void Shader::checkCompileErrors(unsigned int shader, const std::string& type)
{
    int  success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "[Shader] Blad kompilacji (" << type << "):\n" << infoLog << "\n";
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "[Shader] Blad linkowania:\n" << infoLog << "\n";
        }
    }
}
```

### src/core/Camera.h
```cpp
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

// Tryby kamery odpowiadajace trzem trybom aplikacji
enum class CameraMode {
    TOP_DOWN,  // Tryb projektowy: widok z gory, rzutowanie ortograficzne
    FREE,      // Tryb obserwatora: swobodna kamera FPS
    EXPLORE    // Tryb eksploracyjny: chodzenie, Y zablokowane na wysokosci oczu
};

class Camera {
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;

    float moveSpeed;
    float mouseSensitivity;
    float fov;
    float topDownZoom;

    CameraMode mode;

    Camera();

    glm::mat4  getViewMatrix()                   const;
    glm::mat4  getProjectionMatrix(float aspect) const;
    CameraMode getMode()                         const { return mode; }

    void processKeyboard   (GLFWwindow* window, float deltaTime);
    void processMouseMove  (float xOffset, float yOffset);
    void processMouseScroll(float yOffset);
    void setMode(CameraMode newMode);

private:
    void updateVectors();
};
```

### src/core/Camera.cpp
```cpp
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

Camera::Camera()
    : position(0.0f, 8.0f, 0.0f)
    , front(0.0f, -1.0f, 0.0f)
    , up(0.0f, 1.0f, 0.0f)
    , right(1.0f, 0.0f, 0.0f)
    , worldUp(0.0f, 1.0f, 0.0f)
    , yaw(-90.0f)
    , pitch(-89.0f)
    , moveSpeed(5.0f)
    , mouseSensitivity(0.1f)
    , fov(60.0f)
    , topDownZoom(20.0f)
    , mode(CameraMode::TOP_DOWN)
{
    updateVectors();
}

glm::mat4 Camera::getViewMatrix() const
{
    if (mode == CameraMode::TOP_DOWN) {
        return glm::lookAt(
            position,
            glm::vec3(position.x, 0.0f, position.z),
            glm::vec3(0.0f, 0.0f, -1.0f)
        );
    }
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix(float aspect) const
{
    if (mode == CameraMode::TOP_DOWN) {
        float h = topDownZoom * 0.5f;
        float w = h * aspect;
        return glm::ortho(-w, w, -h, h, 0.1f, 200.0f);
    }
    return glm::perspective(glm::radians(fov), aspect, 0.1f, 200.0f);
}

void Camera::processKeyboard(GLFWwindow* window, float deltaTime)
{
    float velocity = moveSpeed * deltaTime;

    if (mode == CameraMode::TOP_DOWN) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) position.z -= velocity;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) position.z += velocity;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) position.x -= velocity;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) position.x += velocity;
    }
    else if (mode == CameraMode::FREE) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) position += front   * velocity;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) position -= front   * velocity;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) position -= right   * velocity;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) position += right   * velocity;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) position += worldUp * velocity;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) position -= worldUp * velocity;
    }
    else if (mode == CameraMode::EXPLORE) {
        glm::vec3 flatFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
        glm::vec3 flatRight = glm::normalize(glm::vec3(right.x, 0.0f, right.z));
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) position += flatFront * velocity;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) position -= flatFront * velocity;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) position -= flatRight * velocity;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) position += flatRight * velocity;
        position.y = 1.7f;  // zablokuj na wysokosci oczu
    }
}

void Camera::processMouseMove(float xOffset, float yOffset)
{
    if (mode == CameraMode::TOP_DOWN) return;
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;
    yaw     += xOffset;
    pitch   += yOffset;
    pitch    = std::clamp(pitch, -89.0f, 89.0f);
    updateVectors();
}

void Camera::processMouseScroll(float yOffset)
{
    if (mode == CameraMode::TOP_DOWN) {
        topDownZoom -= yOffset * 1.0f;
        topDownZoom  = std::clamp(topDownZoom, 3.0f, 80.0f);
    } else {
        fov -= yOffset * 2.0f;
        fov  = std::clamp(fov, 10.0f, 90.0f);
    }
}

void Camera::setMode(CameraMode newMode)
{
    mode = newMode;
    if (newMode == CameraMode::TOP_DOWN) {
        position = glm::vec3(0.0f, 10.0f, 0.0f);
    } else if (newMode == CameraMode::FREE) {
        position = glm::vec3(0.0f, 3.0f, 5.0f);
        yaw      = -90.0f;
        pitch    = -20.0f;
        updateVectors();
    } else if (newMode == CameraMode::EXPLORE) {
        position.y = 1.7f;
        pitch      = 0.0f;
        updateVectors();
    }
}

void Camera::updateVectors()
{
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
    right = glm::normalize(glm::cross(front, worldUp));
    up    = glm::normalize(glm::cross(right, front));
}
```

### src/main.cpp
```cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "core/Shader.h"
#include "core/Camera.h"

#include <iostream>
#include <vector>

const int SCR_WIDTH  = 1280;
const int SCR_HEIGHT = 720;

Camera camera;

float lastMouseX   = SCR_WIDTH  / 2.0f;
float lastMouseY   = SCR_HEIGHT / 2.0f;
bool  firstMouse   = true;
bool  mouseCapured = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{ glViewport(0, 0, width, height); }

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{ camera.processMouseScroll((float)yoffset); }

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (!mouseCapured) return;
    if (firstMouse) {
        lastMouseX = (float)xpos;
        lastMouseY = (float)ypos;
        firstMouse = false;
    }
    float xOffset = (float)xpos - lastMouseX;
    float yOffset = lastMouseY - (float)ypos;
    lastMouseX = (float)xpos;
    lastMouseY = (float)ypos;
    camera.processMouseMove(xOffset, yOffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) return;

    if (key == GLFW_KEY_ESCAPE) {
        if (mouseCapured) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            mouseCapured = false;
            firstMouse   = true;
        } else {
            glfwSetWindowShouldClose(window, true);
        }
    }
    if (key == GLFW_KEY_1) {
        camera.setMode(CameraMode::TOP_DOWN);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        mouseCapured = false;
        std::cout << "[Kamera] TOP DOWN\n";
    }
    if (key == GLFW_KEY_2) {
        camera.setMode(CameraMode::FREE);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        mouseCapured = true; firstMouse = true;
        std::cout << "[Kamera] FREE\n";
    }
    if (key == GLFW_KEY_3) {
        camera.setMode(CameraMode::EXPLORE);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        mouseCapured = true; firstMouse = true;
        std::cout << "[Kamera] EXPLORE\n";
    }
}

unsigned int createGrid(int lines, float size)
{
    std::vector<float> vertices;
    float half = size * 0.5f;
    float step = size / (float)(lines - 1);
    for (int i = 0; i < lines; ++i) {
        float z = -half + i * step;
        vertices.insert(vertices.end(), { -half, 0.0f, z });
        vertices.insert(vertices.end(), {  half, 0.0f, z });
    }
    for (int i = 0; i < lines; ++i) {
        float x = -half + i * step;
        vertices.insert(vertices.end(), { x, 0.0f, -half });
        vertices.insert(vertices.end(), { x, 0.0f,  half });
    }
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    return VAO;
}

unsigned int createFloor(float size)
{
    float h = size * 0.5f;
    float vertices[] = {
        -h, 0.0f, -h,  0.0f,1.0f,0.0f,  0.0f,0.0f,
         h, 0.0f, -h,  0.0f,1.0f,0.0f,  1.0f,0.0f,
         h, 0.0f,  h,  0.0f,1.0f,0.0f,  1.0f,1.0f,
        -h, 0.0f,  h,  0.0f,1.0f,0.0f,  0.0f,1.0f,
    };
    unsigned int indices[] = { 0,1,2, 0,2,3 };
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    return VAO;
}

int main(int argc, char* argv[])
{
    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n"; return -1;
    }
    if (!glfwInit()) {
        std::cerr << "GLFW init failed!\n"; SDL_Quit(); return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,
        "Interior Designer 3D | [1] Top [2] Free [3] Explore", nullptr, nullptr);
    if (!window) {
        std::cerr << "Window creation failed!\n"; glfwTerminate(); SDL_Quit(); return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback      (window, mouse_callback);
    glfwSetScrollCallback         (window, scroll_callback);
    glfwSetKeyCallback            (window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD init failed!\n"; glfwTerminate(); SDL_Quit(); return -1;
    }
    glEnable(GL_DEPTH_TEST);

    std::cout << "OpenGL: " << glGetString(GL_VERSION)  << "\n";
    std::cout << "GPU:    " << glGetString(GL_RENDERER) << "\n";
    std::cout << "\n[1] Top-down  [2] Free cam  [3] Explore\n";
    std::cout << "WASD-ruch | mysz-obrot(2/3) | E/Q-gora/dol(2) | Scroll-zoom | ESC-wyjdz\n\n";

    Shader phongShader("shaders/phong.vert", "shaders/phong.frag");
    Shader flatShader ("shaders/flat.vert",  "shaders/flat.frag");

    unsigned int floorVAO     = createFloor(30.0f);
    unsigned int gridVAO      = createGrid(31, 30.0f);
    int          gridLineCount = 31 * 2 * 2;

    glm::vec3 lightDir      = glm::normalize(glm::vec3(0.5f, 1.0f, 0.3f));
    glm::vec3 lightAmbient  = glm::vec3(0.35f, 0.35f, 0.35f);
    glm::vec3 lightDiffuse  = glm::vec3(0.8f,  0.8f,  0.75f);
    glm::vec3 lightSpecular = glm::vec3(0.3f,  0.3f,  0.3f);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        camera.processKeyboard(window, deltaTime);

        glClearColor(0.15f, 0.15f, 0.18f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float     aspect     = (float)SCR_WIDTH / (float)SCR_HEIGHT;
        glm::mat4 view       = camera.getViewMatrix();
        glm::mat4 projection = camera.getProjectionMatrix(aspect);
        glm::mat4 model      = glm::mat4(1.0f);

        phongShader.use();
        phongShader.setMat4 ("model",          model);
        phongShader.setMat4 ("view",           view);
        phongShader.setMat4 ("projection",     projection);
        phongShader.setVec3 ("viewPos",        camera.position);
        phongShader.setVec3 ("objectColor",    glm::vec3(0.55f, 0.50f, 0.45f));
        phongShader.setVec3 ("lightDirection", lightDir);
        phongShader.setVec3 ("lightAmbient",   lightAmbient);
        phongShader.setVec3 ("lightDiffuse",   lightDiffuse);
        phongShader.setVec3 ("lightSpecular",  lightSpecular);
        phongShader.setFloat("shininess",      32.0f);
        glBindVertexArray(floorVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        flatShader.use();
        flatShader.setMat4("model",      model);
        flatShader.setMat4("view",       view);
        flatShader.setMat4("projection", projection);
        if (camera.getMode() == CameraMode::TOP_DOWN)
            flatShader.setVec4("color", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
        else
            flatShader.setVec4("color", glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));
        glBindVertexArray(gridVAO);
        glDrawArrays(GL_LINES, 0, gridLineCount);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    SDL_Quit();
    return 0;
}
```

---

## WYMAGANIA PROWADZĄCEGO (z pliku PDF)

| # | Wymaganie | Status |
|---|---|---|
| 1 | Inicjalizacja OpenGL + poprawne tworzenie okna | ✅ GOTOWE |
| 2 | Renderowanie obiektów 3D (minimum 3 różne modele) | ⏳ Faza 4 |
| 3 | Rzutowanie perspektywiczne (kamera 3D) | ✅ GOTOWE — Faza 1 |
| 4 | Interakcja z użytkownikiem (mysz/klawiatura) | ✅ GOTOWE — Faza 1 |
| 5 | Źródło światła (ambient + directional) | ✅ GOTOWE — Faza 1 |
| 6 | Cieniowanie Phong | ✅ GOTOWE — Faza 1 |
| 7 | Teksturowanie przynajmniej jednego obiektu | ⏳ Faza 4 |
| 8 | Animacja | ⏳ Faza 3 |
| 9 | Komentarze + dokumentacja PDF | ⏳ Faza 6 |
| 10 | Filtrowanie obrazu | ⏳ Faza 6 |

---

## OPIS APLIKACJI (wymagania funkcjonalne)

Aplikacja do projektowania wnętrza mieszkania w środowisku 2D/3D.
Wszystkie odległości w metrach (możliwość wartości dziesiętnych).

### Tryby kamery
| Tryb | Klawisz | Opis |
|---|---|---|
| **Projektowy** | `1` | Widok z góry (ortho), WASD przesuwa, scroll zoom |
| **Obserwatora** | `2` | Swobodna kamera 3D, mysz obraca, E/Q góra/dół |
| **Eksploracyjny** | `3` | Chodzenie, Y=1.7m, brak latania |

### Tworzenie obrysu mieszkania
- Zamknięty wielokąt 2D, użytkownik podaje: długość, kierunek (±), kąt
- Po zamknięciu: podanie wysokości + wskazanie ściany z drzwiami głównymi
- Punkt startowy = (0,0), po zatwierdzeniu odblokowują się pozostałe funkcje

### Ściany wewnętrzne
- Punkt startowy + grubość + długość + kąt, można łączyć ściany

### Drzwi
- Wybór ściany → położenie → wysokość → typ (tradycyjne/przesuwne)
- Animacja otwierania w trybie eksploracyjnym (zbliżenie)

### Okna
- Wybór ściany → wymiary → położenie, automatyczne wycięcie w geometrii

### Meble
- Biblioteka wbudowana + import OBJ + kreator
- Drag & drop lub wpisanie współrzędnych

### Kreator obiektów (osobne okno ImGui)
- Bryły: sześcian, kula, graniastosłup trójkątny, scalanie w jeden obiekt

### Dodatkowe
- Zapis/wczytywanie (JSON), Undo/Redo, walidacja, eksport

---

## STATUS FAZ

| Faza | Nazwa | Status |
|---|---|---|
| Faza 0 | Setup środowiska | ✅ GOTOWE |
| Faza 1 | Renderer + Kamera + Shadery Phong | ✅ GOTOWE |
| Faza 2 | Obrys mieszkania + ImGui | ⏳ DO ZROBIENIA |
| Faza 3 | Ściany wew. + Drzwi + Okna | ⏳ DO ZROBIENIA |
| Faza 4 | Meble + Tekstury | ⏳ DO ZROBIENIA |
| Faza 5 | Kreator + Import OBJ | ⏳ DO ZROBIENIA |
| Faza 6 | Zapis/Undo/Postprocessing/Docs | ⏳ DO ZROBIENIA |

---

## PLAN FAZ — szczegóły pozostałych

### ⏳ FAZA 2 — Obrys mieszkania + Panel UI (ImGui)
**Do pobrania PRZED fazą:** ImGui → https://github.com/ocornut/imgui → Download ZIP (branch master)
Skopiować do `external/imgui/`: `imgui.h`, `imgui.cpp`, `imgui_draw.cpp`,
`imgui_tables.cpp`, `imgui_widgets.cpp`, `backends/imgui_impl_glfw.h/.cpp`,
`backends/imgui_impl_opengl3.h/.cpp`

Nowe pliki: `src/ui/Panel.h/.cpp`, `src/scene/Outline.h/.cpp`, `src/scene/Wall.h/.cpp`

Algorytm wytłaczania krawędzi → ściana 3D:
```
Krawędź 2D: A(x1,z1) → B(x2,z2), wysokość h
4 wierzchołki: A_bot(x1,0,z1), B_bot(x2,0,z2), B_top(x2,h,z2), A_top(x1,h,z1)
```

### ⏳ FAZA 3 — Ściany wewnętrzne + Drzwi + Okna
Nowe pliki: `src/scene/InnerWall.h/.cpp`, `Door.h/.cpp`, `Window.h/.cpp`
Animacja drzwi (`float openAngle` interpolowany) → spełnia pkt 8 PDF.

### ⏳ FAZA 4 — Meble + Teksturowanie
Do pobrania: `stb_image.h` → https://github.com/nothings/stb → `external/stb/`
Nowe pliki: `src/rendering/Texture.h/.cpp`, `Mesh.h/.cpp`, `Model.h/.cpp`, `src/scene/Furniture.h/.cpp`
Foldery: `assets/textures/`, `assets/models/`
Spełnia pkt 2 i 7 PDF.

### ⏳ FAZA 5 — Kreator obiektów + Import OBJ
Nowe pliki: `src/scene/ObjectCreator.h/.cpp`

### ⏳ FAZA 6 — Zapis/Undo/Postprocessing/Dokumentacja
Do pobrania: `json.hpp` → https://github.com/nlohmann/json → `external/json/`
Nowe pliki: `src/core/SceneSerializer.h/.cpp`, `UndoRedo.h/.cpp`, `src/rendering/PostProcessor.h/.cpp`
Nowe shadery: `shaders/screen.vert`, `shaders/screen.frag`
Spełnia pkt 9 i 10 PDF.

---

## KOMENDY BUDOWANIA

```bash
# Po zmianie CMakeLists.txt:
cd C:\Users\studia\Desktop\proj_graf\build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug

# Kolejne buildy:
mingw32-make

# Uruchomienie:
./OpenGLProject.exe
```

---

*Aby kontynuować projekt w nowym czacie: wklej ten plik i napisz "Jedziemy z fazą X".*
