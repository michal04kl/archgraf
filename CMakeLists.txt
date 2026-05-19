cmake_minimum_required(VERSION 3.16)
project(OpenGLProject VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/core/Shader.cpp
    src/core/Camera.cpp
    src/scene/Wall.cpp
    src/scene/InnerWall.cpp
    src/scene/Door.cpp
    src/scene/RoomWindow.cpp
    src/scene/Furniture.cpp
    src/rendering/Texture.cpp
    src/rendering/Mesh.cpp
    src/rendering/OBJLoader.cpp
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
    external/stb
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
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
)