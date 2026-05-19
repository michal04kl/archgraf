# PROJEKT: Aplikacja do projektowania wnętrz 3D — Dokumentacja Kontekstowa

> Ten plik zawiera WSZYSTKO potrzebne do kontynuowania projektu w nowym czacie.
> Wklej go i napisz: **"Jedziemy z fazą X"** — i od razu lecimy z kodem.

---

## ŚRODOWISKO

- **OS:** Windows 10/11
- **IDE:** Visual Studio Code
- **Kompilator:** MSYS2 → g++ 14.2.0 (ucrt64), ścieżka: `C:/msys64/ucrt64/bin/`
- **Build:** CMake + MinGW Makefiles

### Biblioteki
| Biblioteka | Wersja | Status |
|---|---|---|
| GLAD | Core 3.3 | ✅ |
| GLFW | Windows 64-bit pre-compiled | ✅ |
| SDL3 | 3.4.2 (mingw) | ✅ |
| OpenGL | 3.3 Core Profile | ✅ |
| GLM | 1.0.1 (pełna, light release) | ✅ |
| ImGui | master (najnowszy) | ✅ |

### WAŻNE — SDL3
- `SDL_Init()` zwraca `bool` → `if (!SDL_Init(...))`
- Header: `#include <SDL3/SDL.h>`
- DLL: `SDL3.dll`

---

## STRUKTURA PROJEKTU (po Fazie 2)

```
proj_graf/
├── CMakeLists.txt
├── shaders/
│   ├── phong.vert
│   ├── phong.frag
│   ├── flat.vert
│   └── flat.frag
├── .vscode/
│   ├── c_cpp_properties.json
│   ├── launch.json
│   └── settings.json
├── build/
│   ├── OpenGLProject.exe
│   ├── SDL3.dll
│   └── shaders/              ← kopiowane przez CMake
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── Shader.h / Shader.cpp
│   │   └── Camera.h / Camera.cpp
│   ├── scene/
│   │   ├── Outline.h         ← header-only
│   │   └── Wall.h / Wall.cpp
│   └── ui/
│       ├── Panel.h
│       └── Panel.cpp
└── external/
    ├── glad/
    │   ├── include/glad/glad.h
    │   ├── include/KHR/khrplatform.h
    │   └── src/glad.c
    ├── glfw/
    │   ├── include/GLFW/glfw3.h
    │   ├── include/GLFW/glfw3native.h
    │   └── lib-mingw-w64/libglfw3.a + glfw3.dll + libglfw3dll.a
    ├── glm/glm/               ← PEŁNA biblioteka (detail/, gtc/, gtx/ itd.)
    ├── imgui/
    │   ├── imgui.h / imgui.cpp
    │   ├── imgui_draw.cpp / imgui_tables.cpp / imgui_widgets.cpp
    │   ├── imgui_internal.h / imconfig.h
    │   ├── imstb_rectpack.h / imstb_textedit.h / imstb_truetype.h
    │   └── backends/
    │       ├── imgui_impl_glfw.h/.cpp
    │       ├── imgui_impl_opengl3.h/.cpp
    │       └── imgui_impl_opengl3_loader.h
    └── SDL2/                  ← folder SDL2 zawiera SDL3!
        ├── include/SDL3/SDL.h (+ reszta)
        └── lib/x64/libSDL3.dll.a + SDL3.dll
```

---

## PLIKI KONFIGURACYJNE

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
    src/scene/Wall.cpp
    src/ui/Panel.cpp
    external/glad/src/glad.c
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    external/glad/include
    external/glfw/include
    external/SDL2/include
    external/glm
    external/imgui
    external/imgui/backends
    src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/external/glfw/lib-mingw-w64/libglfw3.a
    ${CMAKE_SOURCE_DIR}/external/SDL2/lib/x64/libSDL3.dll.a
    opengl32 gdi32 winmm imm32 ole32 oleaut32
    version uuid advapi32 setupapi shell32
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/external/SDL2/lib/x64/SDL3.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/shaders"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
)
```

### .vscode/c_cpp_properties.json
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
                "${workspaceFolder}/external/glm",
                "${workspaceFolder}/external/imgui",
                "${workspaceFolder}/external/imgui/backends"
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
    vec3 ambient    = lightAmbient * objectColor;
    vec3 norm       = normalize(Normal);
    vec3 lightDir   = normalize(lightDirection);
    float diff      = max(dot(norm, lightDir), 0.0);
    vec3 diffuse    = lightDiffuse * diff * objectColor;
    vec3 viewDir    = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec      = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular   = lightSpecular * spec;
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
void main() { FragColor = color; }
```

### src/core/Shader.h
```cpp
#pragma once
#include <string>
#include <glm/glm.hpp>

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
    std::string vertexCode, fragmentCode;
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
        std::cerr << "[Shader] Blad odczytu: " << e.what() << "\n";
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
void Shader::setBool (const std::string& n, bool v)          const { glUniform1i (glGetUniformLocation(ID,n.c_str()),(int)v); }
void Shader::setInt  (const std::string& n, int v)           const { glUniform1i (glGetUniformLocation(ID,n.c_str()),v); }
void Shader::setFloat(const std::string& n, float v)         const { glUniform1f (glGetUniformLocation(ID,n.c_str()),v); }
void Shader::setVec2 (const std::string& n, const glm::vec2& v) const { glUniform2fv(glGetUniformLocation(ID,n.c_str()),1,glm::value_ptr(v)); }
void Shader::setVec3 (const std::string& n, const glm::vec3& v) const { glUniform3fv(glGetUniformLocation(ID,n.c_str()),1,glm::value_ptr(v)); }
void Shader::setVec4 (const std::string& n, const glm::vec4& v) const { glUniform4fv(glGetUniformLocation(ID,n.c_str()),1,glm::value_ptr(v)); }
void Shader::setMat4 (const std::string& n, const glm::mat4& m) const { glUniformMatrix4fv(glGetUniformLocation(ID,n.c_str()),1,GL_FALSE,glm::value_ptr(m)); }

void Shader::checkCompileErrors(unsigned int shader, const std::string& type)
{
    int success; char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) { glGetShaderInfoLog(shader,1024,nullptr,infoLog); std::cerr<<"[Shader] "<<type<<":\n"<<infoLog<<"\n"; }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) { glGetProgramInfoLog(shader,1024,nullptr,infoLog); std::cerr<<"[Shader] LINK:\n"<<infoLog<<"\n"; }
    }
}
```

### src/core/Camera.h
```cpp
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

enum class CameraMode { TOP_DOWN, FREE, EXPLORE };

class Camera {
public:
    glm::vec3 position;
    glm::vec3 front, up, right, worldUp;
    float yaw, pitch;
    float moveSpeed, mouseSensitivity, fov, topDownZoom;
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
    : position(0,8,0), front(0,-1,0), up(0,1,0), right(1,0,0), worldUp(0,1,0)
    , yaw(-90), pitch(-89), moveSpeed(5), mouseSensitivity(0.1f)
    , fov(60), topDownZoom(20), mode(CameraMode::TOP_DOWN)
{ updateVectors(); }

glm::mat4 Camera::getViewMatrix() const
{
    if (mode == CameraMode::TOP_DOWN)
        return glm::lookAt(position, glm::vec3(position.x,0,position.z), glm::vec3(0,0,-1));
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix(float aspect) const
{
    if (mode == CameraMode::TOP_DOWN) {
        float h = topDownZoom*0.5f, w = h*aspect;
        return glm::ortho(-w,w,-h,h,0.1f,200.0f);
    }
    return glm::perspective(glm::radians(fov), aspect, 0.1f, 200.0f);
}

void Camera::processKeyboard(GLFWwindow* window, float dt)
{
    float v = moveSpeed * dt;
    if (mode == CameraMode::TOP_DOWN) {
        if (glfwGetKey(window,GLFW_KEY_W)==GLFW_PRESS) position.z -= v;
        if (glfwGetKey(window,GLFW_KEY_S)==GLFW_PRESS) position.z += v;
        if (glfwGetKey(window,GLFW_KEY_A)==GLFW_PRESS) position.x -= v;
        if (glfwGetKey(window,GLFW_KEY_D)==GLFW_PRESS) position.x += v;
    } else if (mode == CameraMode::FREE) {
        if (glfwGetKey(window,GLFW_KEY_W)==GLFW_PRESS) position += front*v;
        if (glfwGetKey(window,GLFW_KEY_S)==GLFW_PRESS) position -= front*v;
        if (glfwGetKey(window,GLFW_KEY_A)==GLFW_PRESS) position -= right*v;
        if (glfwGetKey(window,GLFW_KEY_D)==GLFW_PRESS) position += right*v;
        if (glfwGetKey(window,GLFW_KEY_E)==GLFW_PRESS) position += worldUp*v;
        if (glfwGetKey(window,GLFW_KEY_Q)==GLFW_PRESS) position -= worldUp*v;
    } else if (mode == CameraMode::EXPLORE) {
        glm::vec3 ff = glm::normalize(glm::vec3(front.x,0,front.z));
        glm::vec3 fr = glm::normalize(glm::vec3(right.x,0,right.z));
        if (glfwGetKey(window,GLFW_KEY_W)==GLFW_PRESS) position += ff*v;
        if (glfwGetKey(window,GLFW_KEY_S)==GLFW_PRESS) position -= ff*v;
        if (glfwGetKey(window,GLFW_KEY_A)==GLFW_PRESS) position -= fr*v;
        if (glfwGetKey(window,GLFW_KEY_D)==GLFW_PRESS) position += fr*v;
        position.y = 1.7f;
    }
}

void Camera::processMouseMove(float xO, float yO)
{
    if (mode == CameraMode::TOP_DOWN) return;
    yaw   += xO * mouseSensitivity;
    pitch += yO * mouseSensitivity;
    pitch  = std::clamp(pitch, -89.0f, 89.0f);
    updateVectors();
}

void Camera::processMouseScroll(float yO)
{
    if (mode == CameraMode::TOP_DOWN) { topDownZoom = std::clamp(topDownZoom-yO, 3.0f, 80.0f); }
    else                              { fov         = std::clamp(fov-yO*2.0f, 10.0f, 90.0f); }
}

void Camera::setMode(CameraMode m)
{
    mode = m;
    if (m == CameraMode::TOP_DOWN)  { position = glm::vec3(0,10,0); }
    else if (m == CameraMode::FREE) { position = glm::vec3(0,3,5); yaw=-90; pitch=-20; updateVectors(); }
    else                            { position.y=1.7f; pitch=0; updateVectors(); }
}

void Camera::updateVectors()
{
    glm::vec3 f;
    f.x   = cos(glm::radians(yaw))*cos(glm::radians(pitch));
    f.y   = sin(glm::radians(pitch));
    f.z   = sin(glm::radians(yaw))*cos(glm::radians(pitch));
    front = glm::normalize(f);
    right = glm::normalize(glm::cross(front, worldUp));
    up    = glm::normalize(glm::cross(right, front));
}
```

---

## KOD ŹRÓDŁOWY — FAZA 2

### src/scene/Outline.h
```cpp
#pragma once
#include <vector>
#include <glm/glm.hpp>

class Outline {
public:
    std::vector<glm::vec2> points;
    bool closed = false;
    static constexpr float CLOSE_DIST = 0.5f;

    void addPoint(glm::vec2 p)  { if (!closed) points.push_back(p); }
    void removeLast()           { if (!points.empty() && !closed) points.pop_back(); }
    void clear()                { points.clear(); closed = false; }

    bool tryClose() {
        if (points.size() >= 3) { closed = true; return true; }
        return false;
    }

    void reopen() {
        if (!closed) return;
        closed = false;
        if (points.size() >= 2 && glm::length(points.back()-points[0]) < 0.001f)
            points.pop_back();
    }

    bool isNearStart(glm::vec2 p) const {
        return !points.empty() && glm::length(p-points[0]) < CLOSE_DIST;
    }
    bool canClose() const { return points.size() >= 3 && !closed; }

    glm::vec2 centroid() const {
        glm::vec2 c{};
        for (auto& p : points) c += p;
        return points.empty() ? c : c/(float)points.size();
    }

    float perimeter() const {
        float t = 0;
        for (size_t i = 1; i < points.size(); ++i)
            t += glm::length(points[i]-points[i-1]);
        if (closed && points.size() >= 2)
            t += glm::length(points[0]-points.back());
        return t;
    }

    float edgeLength(int i) const {
        if ((int)points.size() < 2) return 0;
        int j = (i+1)%(int)points.size();
        return glm::length(points[j]-points[i]);
    }
};
```

### src/scene/Wall.h
```cpp
#pragma once
#include <glm/glm.hpp>

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

    void build(glm::vec2 centroid2D);
    void draw() const;
};
```

### src/scene/Wall.cpp
```cpp
#include "Wall.h"
#include <glad/glad.h>
#include <glm/glm.hpp>

Wall::Wall(glm::vec2 s, glm::vec2 e, float h) : start(s), end(e), height(h) {}

Wall::Wall(Wall&& o) noexcept
    : start(o.start), end(o.end), height(o.height)
    , VAO(o.VAO), VBO(o.VBO), vertexCount(o.vertexCount)
{ o.VAO = 0; o.VBO = 0; }

Wall& Wall::operator=(Wall&& o) noexcept {
    if (this != &o) {
        if (VAO) glDeleteVertexArrays(1,&VAO);
        if (VBO) glDeleteBuffers(1,&VBO);
        start=o.start; end=o.end; height=o.height;
        VAO=o.VAO; VBO=o.VBO; vertexCount=o.vertexCount;
        o.VAO=0; o.VBO=0;
    }
    return *this;
}

Wall::~Wall() {
    if (VAO) glDeleteVertexArrays(1,&VAO);
    if (VBO) glDeleteBuffers(1,&VBO);
}

void Wall::build(glm::vec2 centroid2D)
{
    glm::vec2 d = end - start;
    float len   = glm::length(d);
    if (len < 0.001f) return;
    glm::vec2 dir = d / len;
    glm::vec2 n1  = glm::vec2(-dir.y, dir.x);
    glm::vec2 n2  = glm::vec2( dir.y,-dir.x);
    glm::vec2 mid = (start+end)*0.5f;
    glm::vec2 nXZ = (glm::dot(n1, centroid2D-mid) > 0) ? n1 : n2;
    glm::vec3 normal(nXZ.x, 0, nXZ.y);

    glm::vec3 Ab(start.x,0,start.y), Bb(end.x,0,end.y);
    glm::vec3 At(start.x,height,start.y), Bt(end.x,height,end.y);

    float verts[] = {
        Ab.x,Ab.y,Ab.z, normal.x,normal.y,normal.z, 0,0,
        Bb.x,Bb.y,Bb.z, normal.x,normal.y,normal.z, len,0,
        Bt.x,Bt.y,Bt.z, normal.x,normal.y,normal.z, len,1,
        Ab.x,Ab.y,Ab.z, normal.x,normal.y,normal.z, 0,0,
        Bt.x,Bt.y,Bt.z, normal.x,normal.y,normal.z, len,1,
        At.x,At.y,At.z, normal.x,normal.y,normal.z, 0,1,
    };
    vertexCount = 6;

    glGenVertexArrays(1,&VAO); glGenBuffers(1,&VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(verts),verts,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)0);             glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(6*sizeof(float))); glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void Wall::draw() const {
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}
```

### src/ui/Panel.h
```cpp
#pragma once
#include "scene/Outline.h"
#include "core/Camera.h"
#include <glm/glm.hpp>

enum class AppState { DRAWING_OUTLINE, CONFIRM_OUTLINE, DESIGN_MODE };

struct PanelResult {
    bool  undoPoint      = false;
    bool  clearOutline   = false;
    bool  closeOutline   = false;
    bool  confirmOutline = false;
    bool  reopenOutline  = false;
    bool  placeNextPoint = false;   // postaw punkt z length+angle

    float apartmentHeight = 2.7f;
    int   doorWallIndex   = 0;

    float floorSize      = 30.0f;
    bool  floorSizeDirty = false;

    glm::vec2 nextPoint    = {0,0};
    bool      hasNextPoint = false;
};

class Panel {
public:
    AppState state = AppState::DRAWING_OUTLINE;

    float apartmentHeight = 2.7f;
    int   doorWallIndex   = 0;
    float snapGrid        = 0.25f;
    float floorSize       = 30.0f;
    float inputLength     = 1.0f;
    float inputAngle      = 0.0f;

    PanelResult render(const Outline& outline, CameraMode cameraMode);
    glm::vec2 computeNextPoint(const Outline& outline) const;
    float     getAbsoluteAngle(const Outline& outline) const;
};
```

### src/ui/Panel.cpp
```cpp
#include "Panel.h"
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cstdio>
#include <cmath>

float Panel::getAbsoluteAngle(const Outline& outline) const
{
    float inputRad = glm::radians(inputAngle);
    if (outline.points.size() < 2) return inputRad;
    const glm::vec2& prev = outline.points[outline.points.size()-2];
    const glm::vec2& last = outline.points.back();
    return std::atan2((last-prev).y, (last-prev).x) + inputRad;
}

glm::vec2 Panel::computeNextPoint(const Outline& outline) const
{
    if (outline.points.empty() || outline.closed) return {0,0};
    float a = getAbsoluteAngle(outline);
    const glm::vec2& last = outline.points.back();
    return { last.x + inputLength*std::cos(a), last.y + inputLength*std::sin(a) };
}

PanelResult Panel::render(const Outline& outline, CameraMode cameraMode)
{
    PanelResult result;
    result.floorSize = floorSize;
    result.apartmentHeight = apartmentHeight;
    result.doorWallIndex = doorWallIndex;

    if (cameraMode != CameraMode::TOP_DOWN) {
        ImGui::SetNextWindowPos ({10,10}, ImGuiCond_Always);
        ImGui::SetNextWindowSize({240,50}, ImGuiCond_Always);
        ImGuiWindowFlags hf = ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|
            ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoTitleBar|
            ImGuiWindowFlags_NoFocusOnAppearing;
        ImGui::Begin("##hint", nullptr, hf);
        ImGui::TextDisabled("[1] Wroc do trybu projektowego");
        ImGui::End();
        return result;
    }

    ImGui::SetNextWindowPos ({0,0}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({290, ImGui::GetIO().DisplaySize.y}, ImGuiCond_Always);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|
        ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoFocusOnAppearing;
    ImGui::Begin("Interior Designer", nullptr, flags);

    ImGui::TextColored({0.4f,0.8f,1.0f,1.0f}, "INTERIOR DESIGNER 3D");
    ImGui::Separator();
    ImGui::TextDisabled("Kamera: [1] Top  [2] Free  [3] Explore");
    ImGui::Spacing();

    if (state == AppState::DRAWING_OUTLINE)
    {
        ImGui::TextColored({1.0f,0.85f,0.0f,1.0f}, "RYSOWANIE OBRYSU");
        ImGui::Separator();

        // Obszar roboczy
        ImGui::TextDisabled("Obszar roboczy:");
        float prev = floorSize;
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderFloat("##floor", &floorSize, 10, 100, "%.0f x %.0f m");
        if (floorSize != prev) { result.floorSize = floorSize; result.floorSizeDirty = true; }

        ImGui::TextDisabled("Siatka snap:");
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderFloat("##snap", &snapGrid, 0.1f, 2.0f, "%.2f m");
        ImGui::Spacing(); ImGui::Separator();

        int nPts = (int)outline.points.size();

        if (nPts == 0) {
            ImGui::TextColored({0.8f,0.8f,0.4f,1.0f}, "Krok 1: Punkt startowy");
            ImGui::TextWrapped("Kliknij LPM na siatce aby umiescic punkt startowy.");
        } else {
            ImGui::TextColored({0.8f,0.8f,0.4f,1.0f}, "Nastepny odcinek:");
        }
        ImGui::Spacing();

        if (nPts >= 1 && !outline.closed) {
            ImGui::Text("Dlugosc (m):");
            ImGui::SetNextItemWidth(-1);
            ImGui::InputFloat("##len", &inputLength, 0.25f, 1.0f, "%.2f");
            if (inputLength < 0.1f) inputLength = 0.1f;
            ImGui::Spacing();

            if (nPts == 1) {
                ImGui::Text("Kierunek (stopnie od +X):");
                ImGui::TextDisabled("0=prawo  90=dol  180=lewo  -90=gora");
            } else {
                ImGui::Text("Kat wzgledem poprzedniego:");
                ImGui::TextDisabled("0=prosto  90=prawo  -90=lewo  180=zawrot");
            }
            ImGui::SetNextItemWidth(-1);
            ImGui::InputFloat("##ang", &inputAngle, 5, 15, "%.1f deg");
            while (inputAngle >  180.0f) inputAngle -= 360.0f;
            while (inputAngle <= -180.0f) inputAngle += 360.0f;
            ImGui::Spacing();

            glm::vec2 next = computeNextPoint(outline);
            result.hasNextPoint = true;
            result.nextPoint    = next;

            ImGui::TextColored({0.4f,1.0f,0.8f,1.0f},
                "Nastepny punkt: (%.2f, %.2f)", next.x, next.y);
            ImGui::TextDisabled("Absolutny kierunek: %.1f deg",
                glm::degrees(getAbsoluteAngle(outline)));
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Button, {0.2f,0.55f,0.2f,1.0f});
            if (ImGui::Button("Postaw punkt  (SPACJA)", ImVec2(-1,0))) {
                result.placeNextPoint = true;
                result.nextPoint      = next;
            }
            ImGui::PopStyleColor();
            ImGui::Spacing(); ImGui::Separator();
        }

        ImGui::Text("Punkty: %d", nPts);
        if (nPts >= 2) ImGui::Text("Obwod:  %.2f m", outline.perimeter());

        if (nPts >= 2) {
            ImGui::Spacing(); ImGui::TextDisabled("Odcinki:");
            ImGui::BeginChild("segs", {0,70}, true);
            for (int i = 0; i+1 < nPts; ++i) {
                glm::vec2 d = outline.points[i+1]-outline.points[i];
                ImGui::Text("  %d: %.2fm  %.0fdeg", i,
                    glm::length(d), glm::degrees(std::atan2(d.y,d.x)));
            }
            ImGui::EndChild();
        }

        ImGui::Spacing(); ImGui::Separator();
        ImGui::TextWrapped("LPM - postaw punkt (snap)");
        ImGui::TextWrapped("PPM / Z - cofnij");
        ImGui::TextWrapped("SPACJA - postaw z dlugosci/kata");
        ImGui::TextWrapped("ENTER  - zamknij obrys");
        ImGui::Spacing();

        if (ImGui::Button("Cofnij punkt  (PPM / Z)", ImVec2(-1,0))) result.undoPoint = true;

        ImGui::BeginDisabled(!outline.canClose());
        ImGui::PushStyleColor(ImGuiCol_Button, {0.2f,0.6f,0.2f,1.0f});
        if (ImGui::Button("Zamknij obrys  (ENTER)", ImVec2(-1,0))) result.closeOutline = true;
        ImGui::PopStyleColor();
        ImGui::EndDisabled();

        if (ImGui::Button("Wyczysc wszystko", ImVec2(-1,0))) result.clearOutline = true;
    }
    else if (state == AppState::CONFIRM_OUTLINE)
    {
        ImGui::TextColored({0.4f,1.0f,0.4f,1.0f}, "PARAMETRY MIESZKANIA");
        ImGui::Separator();
        ImGui::Text("Obwod:  %.2f m", outline.perimeter());
        ImGui::Text("Punkty: %d", (int)outline.points.size());
        ImGui::Spacing();
        ImGui::TextDisabled("Wysokosc pomieszczen:");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputFloat("##h", &apartmentHeight, 0.1f, 0.5f, "%.2f m");
        apartmentHeight = glm::clamp(apartmentHeight, 1.5f, 6.0f);
        ImGui::Spacing(); ImGui::Separator();
        ImGui::TextColored({1.0f,0.75f,0.2f,1.0f}, "Drzwi glowne:");
        ImGui::TextDisabled("(geometria drzwi - Faza 3)");

        int nEdges = (int)outline.points.size();
        doorWallIndex = glm::clamp(doorWallIndex, 0, nEdges-1);
        char lbl[64];
        snprintf(lbl, sizeof(lbl), "Sciana %d  (%.1f m)", doorWallIndex, outline.edgeLength(doorWallIndex));
        ImGui::SetNextItemWidth(-1);
        if (ImGui::BeginCombo("##door", lbl)) {
            for (int i = 0; i < nEdges; ++i) {
                char buf[64];
                snprintf(buf, sizeof(buf), "Sciana %d  (%.1f m)", i, outline.edgeLength(i));
                bool sel = (doorWallIndex==i);
                if (ImGui::Selectable(buf,sel)) doorWallIndex=i;
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::Spacing(); ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Button, {0.2f,0.5f,0.9f,1.0f});
        if (ImGui::Button("Zatwierdz i generuj sciany", ImVec2(-1,0))) {
            result.confirmOutline  = true;
            result.apartmentHeight = apartmentHeight;
            result.doorWallIndex   = doorWallIndex;
        }
        ImGui::PopStyleColor();
        ImGui::Spacing();
        if (ImGui::Button("Wroc do edycji obrysu", ImVec2(-1,0))) result.reopenOutline = true;
    }
    else if (state == AppState::DESIGN_MODE)
    {
        ImGui::TextColored({0.4f,1.0f,0.4f,1.0f}, "TRYB PROJEKTOWANIA");
        ImGui::Separator();
        ImGui::Text("Sciany: %d",    (int)outline.points.size());
        ImGui::Text("Wys.:   %.2f m", apartmentHeight);
        ImGui::Spacing();
        ImGui::TextDisabled("--- Faza 3 ---");
        ImGui::TextDisabled("Sciany wewnetrzne");
        ImGui::TextDisabled("Drzwi i okna");
        ImGui::TextDisabled("--- Faza 4 ---");
        ImGui::TextDisabled("Meble i tekstury");
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Button, {0.7f,0.4f,0.1f,1.0f});
        if (ImGui::Button("Edytuj obrys", ImVec2(-1,0))) result.reopenOutline = true;
        ImGui::PopStyleColor();
    }

    ImGui::End();
    result.apartmentHeight = apartmentHeight;
    result.doorWallIndex   = doorWallIndex;
    return result;
}
```

### src/main.cpp
```cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "core/Shader.h"
#include "core/Camera.h"
#include "scene/Outline.h"
#include "scene/Wall.h"
#include "ui/Panel.h"
#include <iostream>
#include <vector>
#include <cmath>

const int SCR_WIDTH = 1280, SCR_HEIGHT = 720;

Camera camera;
float deltaTime = 0, lastFrame = 0;
float lastMouseX = SCR_WIDTH/2.0f, lastMouseY = SCR_HEIGHT/2.0f;
bool  firstMouse = true, mouseCapured = false;
double cursorScreenX = SCR_WIDTH/2.0, cursorScreenY = SCR_HEIGHT/2.0;
bool pendingLeftClick = false, pendingRightClick = false, pendingSpace = false;

Outline outline;
std::vector<Wall> walls;
Panel panel;

unsigned int outlineLinesVAO=0,outlineLinesVBO=0;
unsigned int outlinePointsVAO=0,outlinePointsVBO=0;
unsigned int outlineFillVAO=0,outlineFillVBO=0;
unsigned int previewLineVAO=0,previewLineVBO=0;
unsigned int anglePreviewVAO=0,anglePreviewVBO=0;
unsigned int snapDotVAO=0,snapDotVBO=0;
unsigned int angleDotVAO=0,angleDotVBO=0;
int outlineLinesCount=0,outlinePointsCount=0,outlineFillCount=0;
bool outlineGLDirty=false;

unsigned int floorVAO=0,floorVBO=0,floorEBO=0;
unsigned int gridVAO=0;
int gridCount=0;
float currentFloorSize=30.0f;

static constexpr float PLAYER_RADIUS = 0.25f;

bool pointInPolygon(glm::vec2 p, const std::vector<glm::vec2>& poly)
{
    if (poly.size()<3) return false;
    bool inside=false; int n=(int)poly.size();
    for (int i=0,j=n-1;i<n;j=i++) {
        float xi=poly[i].x,yi=poly[i].y,xj=poly[j].x,yj=poly[j].y;
        if (((yi>p.y)!=(yj>p.y))&&(p.x<(xj-xi)*(p.y-yi)/(yj-yi)+xi)) inside=!inside;
    }
    return inside;
}

glm::vec2 segmentPushout(glm::vec2 p, glm::vec2 A, glm::vec2 B, float r)
{
    glm::vec2 AB=B-A; float len=glm::length(AB);
    if (len<0.001f) return {0,0};
    float t=glm::clamp(glm::dot(p-A,AB/len),0.0f,len);
    glm::vec2 cl=A+(AB/len)*t, d=p-cl; float dist=glm::length(d);
    if (dist<r&&dist>0.0001f) return glm::normalize(d)*(r-dist);
    return {0,0};
}

glm::vec2 resolveCollisions(glm::vec2 pos)
{
    int n=(int)outline.points.size(); if (n<3) return pos;
    for (int it=0;it<3;++it)
        for (int i=0;i<n;++i)
            pos+=segmentPushout(pos,outline.points[i],outline.points[(i+1)%n],PLAYER_RADIUS);
    return pos;
}

glm::vec2 screenToWorld(double sx, double sy)
{
    float ndcX=(float)(2.0*sx/SCR_WIDTH)-1.0f;
    float ndcY=1.0f-(float)(2.0*sy/SCR_HEIGHT);
    float halfH=camera.topDownZoom*0.5f, halfW=halfH*(float)SCR_WIDTH/SCR_HEIGHT;
    return {camera.position.x+ndcX*halfW, camera.position.z-ndcY*halfH};
}

glm::vec2 snapToGrid(glm::vec2 p, float g)
{ return {std::round(p.x/g)*g, std::round(p.y/g)*g}; }

unsigned int makeSimpleVAO(unsigned int& vbo, size_t n, GLenum usage)
{
    unsigned int vao;
    glGenVertexArrays(1,&vao); glGenBuffers(1,&vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,n*sizeof(float),nullptr,usage);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    return vao;
}

static void deleteVAOVBO(unsigned int& vao, unsigned int& vbo)
{
    if (vao){glDeleteVertexArrays(1,&vao);vao=0;}
    if (vbo){glDeleteBuffers(1,&vbo);vbo=0;}
}

void buildFloor(float size)
{
    if(floorVAO)glDeleteVertexArrays(1,&floorVAO);
    if(floorVBO)glDeleteBuffers(1,&floorVBO);
    if(floorEBO)glDeleteBuffers(1,&floorEBO);
    float h=size*0.5f;
    float v[]={-h,0,-h,0,1,0,0,0, h,0,-h,0,1,0,1,0, h,0,h,0,1,0,1,1, -h,0,h,0,1,0,0,1};
    unsigned int idx[]={0,1,2,0,2,3};
    glGenVertexArrays(1,&floorVAO);glGenBuffers(1,&floorVBO);glGenBuffers(1,&floorEBO);
    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER,floorVBO);glBufferData(GL_ARRAY_BUFFER,sizeof(v),v,GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,floorEBO);glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(idx),idx,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)0);glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(6*sizeof(float)));glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    currentFloorSize=size;
}

void buildGrid(float size)
{
    if(gridVAO)glDeleteVertexArrays(1,&gridVAO);
    std::vector<float> v; float half=size*0.5f;
    for(float z=-half;z<=half+0.001f;z+=1.0f){v.insert(v.end(),{-half,0,z,half,0,z});}
    for(float x=-half;x<=half+0.001f;x+=1.0f){v.insert(v.end(),{x,0,-half,x,0,half});}
    gridCount=(int)(v.size()/3);
    unsigned int vbo;
    glGenVertexArrays(1,&gridVAO);glGenBuffers(1,&vbo);
    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);glBufferData(GL_ARRAY_BUFFER,v.size()*sizeof(float),v.data(),GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void rebuildOutlineGL()
{
    deleteVAOVBO(outlineLinesVAO,outlineLinesVBO);
    std::vector<float> lv;
    for(size_t i=0;i+1<outline.points.size();++i){
        lv.insert(lv.end(),{outline.points[i].x,0.01f,outline.points[i].y});
        lv.insert(lv.end(),{outline.points[i+1].x,0.01f,outline.points[i+1].y});
    }
    if(outline.closed&&outline.points.size()>=2){
        lv.insert(lv.end(),{outline.points.back().x,0.01f,outline.points.back().y});
        lv.insert(lv.end(),{outline.points[0].x,0.01f,outline.points[0].y});
    }
    outlineLinesCount=(int)(lv.size()/3);
    if(!lv.empty()){
        outlineLinesVAO=makeSimpleVAO(outlineLinesVBO,lv.size(),GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,outlineLinesVBO);
        glBufferData(GL_ARRAY_BUFFER,lv.size()*sizeof(float),lv.data(),GL_STATIC_DRAW);
    }
    deleteVAOVBO(outlinePointsVAO,outlinePointsVBO);
    std::vector<float> pv;
    for(auto& p:outline.points) pv.insert(pv.end(),{p.x,0.02f,p.y});
    outlinePointsCount=(int)(pv.size()/3);
    if(!pv.empty()){
        outlinePointsVAO=makeSimpleVAO(outlinePointsVBO,pv.size(),GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,outlinePointsVBO);
        glBufferData(GL_ARRAY_BUFFER,pv.size()*sizeof(float),pv.data(),GL_STATIC_DRAW);
    }
    deleteVAOVBO(outlineFillVAO,outlineFillVBO);
    std::vector<float> fv;
    if(outline.points.size()>=3){
        const glm::vec2& p0=outline.points[0];
        for(size_t i=1;i+1<outline.points.size();++i){
            fv.insert(fv.end(),{p0.x,0.005f,p0.y});
            fv.insert(fv.end(),{outline.points[i].x,0.005f,outline.points[i].y});
            fv.insert(fv.end(),{outline.points[i+1].x,0.005f,outline.points[i+1].y});
        }
    }
    outlineFillCount=(int)(fv.size()/3);
    if(!fv.empty()){
        outlineFillVAO=makeSimpleVAO(outlineFillVBO,fv.size(),GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,outlineFillVBO);
        glBufferData(GL_ARRAY_BUFFER,fv.size()*sizeof(float),fv.data(),GL_STATIC_DRAW);
    }
}

void updateMousePreview(glm::vec2 snap)
{
    if(!outline.points.empty()&&!outline.closed){
        float d[]={outline.points.back().x,0.01f,outline.points.back().y,snap.x,0.01f,snap.y};
        glBindBuffer(GL_ARRAY_BUFFER,previewLineVBO);
        glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(d),d);
    }
    float dd[]={snap.x,0.02f,snap.y};
    glBindBuffer(GL_ARRAY_BUFFER,snapDotVBO);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(dd),dd);
}

void updateAnglePreview(glm::vec2 next, bool valid)
{
    if(!valid||outline.points.empty()||outline.closed) return;
    float d[]={outline.points.back().x,0.015f,outline.points.back().y,next.x,0.015f,next.y};
    glBindBuffer(GL_ARRAY_BUFFER,anglePreviewVBO);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(d),d);
    float dd[]={next.x,0.025f,next.y};
    glBindBuffer(GL_ARRAY_BUFFER,angleDotVBO);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(dd),dd);
}

void buildWalls(float height)
{
    walls.clear();
    glm::vec2 c=outline.centroid();
    for(size_t i=0;i<outline.points.size();++i){
        size_t j=(i+1)%outline.points.size();
        Wall w(outline.points[i],outline.points[j],height);
        w.build(c); walls.push_back(std::move(w));
    }
    std::cout<<"[Scene] Sciany: "<<walls.size()<<"\n";
}

void tryAddPoint(glm::vec2 pos)
{
    if(panel.state!=AppState::DRAWING_OUTLINE) return;
    if(outline.isNearStart(pos)&&outline.canClose()){
        outline.tryClose(); panel.state=AppState::CONFIRM_OUTLINE; outlineGLDirty=true;
    } else { outline.addPoint(pos); outlineGLDirty=true; }
}

void framebuffer_size_callback(GLFWwindow*,int w,int h){glViewport(0,0,w,h);}
void scroll_callback(GLFWwindow*,double,double y){camera.processMouseScroll((float)y);}

void mouse_callback(GLFWwindow*,double xpos,double ypos)
{
    cursorScreenX=xpos; cursorScreenY=ypos;
    if(!mouseCapured) return;
    if(firstMouse){lastMouseX=(float)xpos;lastMouseY=(float)ypos;firstMouse=false;}
    camera.processMouseMove((float)xpos-lastMouseX, lastMouseY-(float)ypos);
    lastMouseX=(float)xpos; lastMouseY=(float)ypos;
}

void mouse_button_callback(GLFWwindow*,int btn,int action,int)
{
    if(action==GLFW_PRESS){
        if(btn==GLFW_MOUSE_BUTTON_LEFT)  pendingLeftClick=true;
        if(btn==GLFW_MOUSE_BUTTON_RIGHT) pendingRightClick=true;
    }
}

void key_callback(GLFWwindow* window,int key,int,int action,int)
{
    if(action!=GLFW_PRESS) return;
    if(key==GLFW_KEY_ESCAPE){
        if(mouseCapured){glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_NORMAL);mouseCapured=false;firstMouse=true;}
        else glfwSetWindowShouldClose(window,true);
    }
    if(key==GLFW_KEY_1){camera.setMode(CameraMode::TOP_DOWN);glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_NORMAL);mouseCapured=false;}
    if(key==GLFW_KEY_2){
        if(panel.state!=AppState::DESIGN_MODE) std::cout<<"[Kamera] Najpierw zatwierdz obrys!\n";
        else{camera.setMode(CameraMode::FREE);glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);mouseCapured=true;firstMouse=true;}
    }
    if(key==GLFW_KEY_3){
        if(panel.state!=AppState::DESIGN_MODE) std::cout<<"[Kamera] Najpierw zatwierdz obrys!\n";
        else{
            glm::vec2 c=outline.centroid();
            camera.setMode(CameraMode::EXPLORE);
            camera.position=glm::vec3(c.x,1.7f,c.y);
            glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
            mouseCapured=true;firstMouse=true;
        }
    }
    if(key==GLFW_KEY_SPACE&&!ImGui::GetIO().WantTextInput) pendingSpace=true;
    if((key==GLFW_KEY_ENTER||key==GLFW_KEY_KP_ENTER)&&!ImGui::GetIO().WantTextInput){
        if(panel.state==AppState::DRAWING_OUTLINE&&outline.canClose()){
            outline.tryClose();panel.state=AppState::CONFIRM_OUTLINE;outlineGLDirty=true;
        }
    }
    if(key==GLFW_KEY_Z&&!ImGui::GetIO().WantTextInput&&panel.state==AppState::DRAWING_OUTLINE){
        outline.removeLast();outlineGLDirty=true;
    }
}

int main(int argc, char* argv[])
{
    if(!SDL_Init(SDL_INIT_AUDIO|SDL_INIT_EVENTS)){std::cerr<<"SDL failed\n";return -1;}
    if(!glfwInit()){std::cerr<<"GLFW failed\n";SDL_Quit();return -1;}
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window=glfwCreateWindow(SCR_WIDTH,SCR_HEIGHT,"Interior Designer 3D",nullptr,nullptr);
    if(!window){std::cerr<<"Window failed\n";glfwTerminate();SDL_Quit();return -1;}
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);
    glfwSetCursorPosCallback(window,mouse_callback);
    glfwSetScrollCallback(window,scroll_callback);
    glfwSetKeyCallback(window,key_callback);
    glfwSetMouseButtonCallback(window,mouse_button_callback);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){std::cerr<<"GLAD failed\n";glfwTerminate();SDL_Quit();return -1;}

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window,true);
    ImGui_ImplOpenGL3_Init("#version 330");

    std::cout<<"OpenGL: "<<glGetString(GL_VERSION)<<"\n";
    std::cout<<"GPU:    "<<glGetString(GL_RENDERER)<<"\n";

    Shader phongShader("shaders/phong.vert","shaders/phong.frag");
    Shader flatShader ("shaders/flat.vert", "shaders/flat.frag");

    buildFloor(currentFloorSize);
    buildGrid(currentFloorSize);

    // Dynamiczne VAO — kursor myszy
    {
        previewLineVAO=makeSimpleVAO(previewLineVBO,6,GL_DYNAMIC_DRAW);
        float ph[]={0,0,0,0,0,0};
        glBindBuffer(GL_ARRAY_BUFFER,previewLineVBO);glBufferData(GL_ARRAY_BUFFER,sizeof(ph),ph,GL_DYNAMIC_DRAW);
        snapDotVAO=makeSimpleVAO(snapDotVBO,3,GL_DYNAMIC_DRAW);
        float pd[]={0,0,0};
        glBindBuffer(GL_ARRAY_BUFFER,snapDotVBO);glBufferData(GL_ARRAY_BUFFER,sizeof(pd),pd,GL_DYNAMIC_DRAW);
    }
    // Dynamiczne VAO — podglad length+angle
    {
        anglePreviewVAO=makeSimpleVAO(anglePreviewVBO,6,GL_DYNAMIC_DRAW);
        float ph[]={0,0,0,0,0,0};
        glBindBuffer(GL_ARRAY_BUFFER,anglePreviewVBO);glBufferData(GL_ARRAY_BUFFER,sizeof(ph),ph,GL_DYNAMIC_DRAW);
        angleDotVAO=makeSimpleVAO(angleDotVBO,3,GL_DYNAMIC_DRAW);
        float pd[]={0,0,0};
        glBindBuffer(GL_ARRAY_BUFFER,angleDotVBO);glBufferData(GL_ARRAY_BUFFER,sizeof(pd),pd,GL_DYNAMIC_DRAW);
    }

    glm::vec3 lightDir=glm::normalize(glm::vec3(0.5f,1.0f,0.3f));
    glm::vec3 lightAmbient(0.35f),lightDiffuse(0.8f,0.8f,0.75f),lightSpecular(0.3f);

    auto setPhong=[&](const glm::mat4& model,glm::vec3 color){
        phongShader.setMat4("model",model);
        phongShader.setVec3("objectColor",color);
        phongShader.setVec3("lightDirection",lightDir);
        phongShader.setVec3("lightAmbient",lightAmbient);
        phongShader.setVec3("lightDiffuse",lightDiffuse);
        phongShader.setVec3("lightSpecular",lightSpecular);
        phongShader.setFloat("shininess",32.0f);
    };

    while(!glfwWindowShouldClose(window))
    {
        float now=(float)glfwGetTime();
        deltaTime=now-lastFrame; lastFrame=now;

        if(camera.getMode()==CameraMode::EXPLORE&&panel.state==AppState::DESIGN_MODE&&outline.points.size()>=3){
            glm::vec3 oldPos=camera.position;
            camera.processKeyboard(window,deltaTime);
            glm::vec2 resolved=resolveCollisions({camera.position.x,camera.position.z});
            if(!pointInPolygon(resolved,outline.points)){camera.position.x=oldPos.x;camera.position.z=oldPos.z;}
            else{camera.position.x=resolved.x;camera.position.z=resolved.y;}
            camera.position.y=1.7f;
        } else camera.processKeyboard(window,deltaTime);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glm::vec2 rawCursor=screenToWorld(cursorScreenX,cursorScreenY);
        glm::vec2 snapCursor=snapToGrid(rawCursor,panel.snapGrid);
        if(!outline.points.empty()&&outline.isNearStart(snapCursor)) snapCursor=outline.points[0];

        PanelResult pr=panel.render(outline,camera.getMode());

        if(!ImGui::GetIO().WantCaptureMouse){
            if(pendingLeftClick&&panel.state==AppState::DRAWING_OUTLINE&&camera.getMode()==CameraMode::TOP_DOWN)
                tryAddPoint(snapCursor);
            if(pendingRightClick&&panel.state==AppState::DRAWING_OUTLINE){outline.removeLast();outlineGLDirty=true;}
        }
        pendingLeftClick=false; pendingRightClick=false;

        if((pendingSpace||pr.placeNextPoint)&&panel.state==AppState::DRAWING_OUTLINE&&pr.hasNextPoint){
            tryAddPoint(pr.nextPoint);
            panel.inputAngle=0.0f;
        }
        pendingSpace=false;

        if(pr.undoPoint){outline.removeLast();outlineGLDirty=true;}
        if(pr.clearOutline){outline.clear();walls.clear();panel.state=AppState::DRAWING_OUTLINE;outlineGLDirty=true;}
        if(pr.closeOutline&&outline.canClose()){outline.tryClose();panel.state=AppState::CONFIRM_OUTLINE;outlineGLDirty=true;}
        if(pr.confirmOutline){buildWalls(pr.apartmentHeight);panel.state=AppState::DESIGN_MODE;}
        if(pr.reopenOutline){
            outline.reopen();walls.clear();panel.state=AppState::DRAWING_OUTLINE;outlineGLDirty=true;
            camera.setMode(CameraMode::TOP_DOWN);glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_NORMAL);mouseCapured=false;
        }
        if(pr.floorSizeDirty){buildFloor(pr.floorSize);buildGrid(pr.floorSize);}
        if(outlineGLDirty){rebuildOutlineGL();outlineGLDirty=false;}
        if(panel.state==AppState::DRAWING_OUTLINE){
            updateMousePreview(snapCursor);
            updateAnglePreview(pr.nextPoint,pr.hasNextPoint);
        }

        glClearColor(0.12f,0.12f,0.14f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        float aspect=(float)SCR_WIDTH/SCR_HEIGHT;
        glm::mat4 view=camera.getViewMatrix();
        glm::mat4 proj=camera.getProjectionMatrix(aspect);
        glm::mat4 I=glm::mat4(1.0f);

        auto setFlatMVP=[&](){
            flatShader.setMat4("model",I);
            flatShader.setMat4("view",view);
            flatShader.setMat4("projection",proj);
        };

        phongShader.use();
        phongShader.setMat4("view",view);phongShader.setMat4("projection",proj);
        phongShader.setVec3("viewPos",camera.position);
        setPhong(I,glm::vec3(0.55f,0.50f,0.45f));
        glBindVertexArray(floorVAO);glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);

        flatShader.use();setFlatMVP();
        float gc=(camera.getMode()==CameraMode::TOP_DOWN)?0.45f:0.25f;
        flatShader.setVec4("color",{gc,gc,gc,1});
        glBindVertexArray(gridVAO);glDrawArrays(GL_LINES,0,gridCount);

        if(!walls.empty()){
            phongShader.use();
            phongShader.setMat4("view",view);phongShader.setMat4("projection",proj);
            phongShader.setVec3("viewPos",camera.position);
            setPhong(I,glm::vec3(0.88f,0.85f,0.80f));
            for(auto& w:walls) w.draw();
        }

        flatShader.use();setFlatMVP();

        if(outlineFillCount>0){
            flatShader.setVec4("color",{0.3f,0.6f,1.0f,0.12f});
            glBindVertexArray(outlineFillVAO);glDrawArrays(GL_TRIANGLES,0,outlineFillCount);
        }
        if(outlineLinesCount>0){
            glLineWidth(2.0f);flatShader.setVec4("color",{1.0f,0.85f,0.0f,1.0f});
            glBindVertexArray(outlineLinesVAO);glDrawArrays(GL_LINES,0,outlineLinesCount);glLineWidth(1.0f);
        }
        if(outlinePointsCount>0){
            glPointSize(9.0f);flatShader.setVec4("color",{1.0f,0.45f,0.0f,1.0f});
            glBindVertexArray(outlinePointsVAO);glDrawArrays(GL_POINTS,0,outlinePointsCount);glPointSize(1.0f);
        }
        if(panel.state==AppState::DRAWING_OUTLINE&&!outline.points.empty()&&!outline.closed){
            glLineWidth(1.0f);flatShader.setVec4("color",{0.7f,0.7f,0.7f,0.4f});
            glBindVertexArray(previewLineVAO);glDrawArrays(GL_LINES,0,2);
        }
        if(panel.state==AppState::DRAWING_OUTLINE&&pr.hasNextPoint&&!outline.points.empty()&&!outline.closed){
            glLineWidth(2.0f);flatShader.setVec4("color",{0.2f,0.6f,1.0f,0.9f});
            glBindVertexArray(anglePreviewVAO);glDrawArrays(GL_LINES,0,2);glLineWidth(1.0f);
            glPointSize(11.0f);flatShader.setVec4("color",{0.2f,0.8f,1.0f,1.0f});
            glBindVertexArray(angleDotVAO);glDrawArrays(GL_POINTS,0,1);glPointSize(1.0f);
        }
        if(panel.state==AppState::DRAWING_OUTLINE&&camera.getMode()==CameraMode::TOP_DOWN){
            glPointSize(9.0f);
            bool ns=outline.isNearStart(snapCursor)&&outline.canClose();
            flatShader.setVec4("color",ns?glm::vec4(0.1f,1.0f,0.3f,1.0f):glm::vec4(0.0f,0.9f,1.0f,0.7f));
            glBindVertexArray(snapDotVAO);glDrawArrays(GL_POINTS,0,1);glPointSize(1.0f);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    walls.clear();
    if(floorVAO)glDeleteVertexArrays(1,&floorVAO);
    if(floorVBO)glDeleteBuffers(1,&floorVBO);
    if(floorEBO)glDeleteBuffers(1,&floorEBO);
    if(gridVAO)glDeleteVertexArrays(1,&gridVAO);
    deleteVAOVBO(outlineLinesVAO,outlineLinesVBO);
    deleteVAOVBO(outlinePointsVAO,outlinePointsVBO);
    deleteVAOVBO(outlineFillVAO,outlineFillVBO);
    deleteVAOVBO(previewLineVAO,previewLineVBO);
    deleteVAOVBO(snapDotVAO,snapDotVBO);
    deleteVAOVBO(anglePreviewVAO,anglePreviewVBO);
    deleteVAOVBO(angleDotVAO,angleDotVBO);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    SDL_Quit();
    return 0;
}
```

---

## WYMAGANIA PDF — STATUS

| # | Wymaganie | Status |
|---|---|---|
| 1 | Inicjalizacja OpenGL + okno | ✅ GOTOWE |
| 2 | Min. 3 modele 3D | ⏳ Faza 4 |
| 3 | Rzutowanie perspektywiczne | ✅ GOTOWE — Faza 1 |
| 4 | Interakcja mysz/klawiatura | ✅ GOTOWE — Faza 1+2 |
| 5 | Oświetlenie ambient+directional | ✅ GOTOWE — Faza 1 |
| 6 | Cieniowanie Phong | ✅ GOTOWE — Faza 1 |
| 7 | Teksturowanie | ⏳ Faza 4 |
| 8 | Animacja | ⏳ Faza 3 |
| 9 | Dokumentacja PDF | ⏳ Faza 6 |
| 10 | Filtrowanie obrazu | ⏳ Faza 6 |

---

## CO DZIAŁA PO FAZIE 2

**Sterowanie:**
| Klawisz | Akcja |
|---|---|
| `1` | Top-down (zawsze dostępna) |
| `2` | Free cam (tylko po zatwierdzeniu obrysu) |
| `3` | Explore — start w środku mieszkania (tylko po zatwierdzeniu) |
| `SPACJA` | Postaw punkt z wpisanej długości i kąta |
| `ENTER` | Zamknij obrys (min 3 punkty) |
| `Z` / PPM | Cofnij ostatni punkt |
| `ESC` | Zwolnij kursor / zamknij |

**Rysowanie obrysu:**
- LPM stawia punkt w miejscu kursora (snap do siatki)
- Pola `Długość` + `Kąt` w panelu + SPACJA = precyzyjne stawianie
- Pierwszy odcinek: kąt absolutny od osi +X (0°=prawo, 90°=dół, 180°=lewo)
- Kolejne: kąt względem poprzedniego odcinka (0°=prosto, 90°=prawo, -90°=lewo)
- Niebieska linia = podgląd gdzie wyląduje punkt z długości+kąta
- Szara linia = podgląd do kursora myszy
- Snap wizualny do pierwszego punktu (zielony dot) — zamknięcie przez LPM lub ENTER

**Po zatwierdzeniu:**
- Generowane ściany 3D z oświetleniem Phong
- Kolizje w trybie EXPLORE — nie można przejść przez ściany

**Panel:**
- Suwak rozmiaru obszaru roboczego (10–100m) — przebudowuje podłogę i siatkę
- Suwak snap grid (0.1–2.0m)
- Widoczny tylko w trybie TOP_DOWN (w trybach 2/3 mały hint w rogu)

---

## STATUS FAZ

| Faza | Nazwa | Status |
|---|---|---|
| Faza 0 | Setup środowiska | ✅ GOTOWE |
| Faza 1 | Renderer + Kamera + Shadery Phong | ✅ GOTOWE |
| Faza 2 | Obrys mieszkania + ImGui | ✅ GOTOWE |
| Faza 3 | Ściany wew. + Drzwi + Okna | ⏳ DO ZROBIENIA |
| Faza 4 | Meble + Tekstury | ⏳ DO ZROBIENIA |
| Faza 5 | Kreator + Import OBJ | ⏳ DO ZROBIENIA |
| Faza 6 | Zapis/Undo/Postprocessing/Docs | ⏳ DO ZROBIENIA |

---

## PLAN KOLEJNYCH FAZ

### ⏳ FAZA 3 — Ściany wewnętrzne + Drzwi + Okna
Nowe pliki: `src/scene/InnerWall.h/.cpp`, `Door.h/.cpp`, `Window.h/.cpp`
- Ściany wewnętrzne: punkt startowy + długość + kąt + grubość → prostopadłościan
- Drzwi: wycięcie w ścianie + animowany quad (`float openAngle`) → spełnia pkt 8 PDF
- Okna: wycięcie w ścianie na danej wysokości + przezroczysty quad

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

*Aby kontynuować w nowym czacie: wklej ten plik i napisz "Jedziemy z fazą X".*
