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

### WAŻNE — SDL3 różnice od SDL2
- `SDL_Init()` zwraca `bool`, nie `int` → używamy `if (!SDL_Init(...))`
- Brak `libSDL3main.a` — SDL3 obsługuje main inaczej
- Header: `#include <SDL3/SDL.h>` (nie `<SDL2/SDL.h>`)
- DLL: `SDL3.dll` (nie `SDL2.dll`)

---

## STRUKTURA PROJEKTU NA DYSKU

```
C:\Users\studia\Desktop\proj_graf\
├── CMakeLists.txt
├── build/                          ← tu leci cmake i mingw32-make
│   └── OpenGLProject.exe          ← gotowy executable (+ SDL3.dll obok)
├── .vscode/
│   ├── settings.json
│   └── launch.json
├── src/
│   └── main.cpp                   ← punkt startowy
└── external/
    ├── glad/
    │   ├── include/
    │   │   ├── glad/glad.h
    │   │   └── KHR/khrplatform.h
    │   └── src/glad.c
    ├── glfw/
    │   ├── include/GLFW/
    │   │   ├── glfw3.h
    │   │   └── glfw3native.h
    │   └── lib-mingw-w64/
    │       ├── glfw3.dll
    │       ├── libglfw3.a
    │       └── libglfw3dll.a
    └── SDL2/                       ← UWAGA: folder nazywa się SDL2 ale zawiera SDL3!
        ├── include/SDL3/
        │   └── SDL.h (+ reszta headerów SDL3)
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
    external/glad/src/glad.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    external/glad/include
    external/glfw/include
    external/SDL2/include
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

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/external/SDL2/lib/x64/SDL3.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
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

### src/main.cpp (aktualny, działający)
```cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <SDL3/SDL.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (!glfwInit()) {
        std::cerr << "GLFW init failed!" << std::endl;
        SDL_Quit();
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL + GLAD + GLFW + SDL3", nullptr, nullptr);
    if (!window) {
        std::cerr << "Window creation failed!" << std::endl;
        glfwTerminate();
        SDL_Quit();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD init failed!" << std::endl;
        glfwTerminate();
        SDL_Quit();
        return -1;
    }

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Renderer:       " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "SDL version:    " << SDL_GetRevision() << std::endl;

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClearColor(0.1f, 0.15f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

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

Projekt musi zawierać **obowiązkowo punkty 1–7**, opcjonalnie 8–10:

| # | Wymaganie | Status |
|---|---|---|
| 1 | Inicjalizacja OpenGL + poprawne tworzenie okna (GLFW/SDL) | ✅ **GOTOWE** |
| 2 | Renderowanie obiektów 3D (minimum 3 różne modele) | ⏳ Faza 4 |
| 3 | Rzutowanie perspektywiczne (kamera 3D) | ⏳ Faza 1 |
| 4 | Interakcja z użytkownikiem (mysz/klawiatura) | ⏳ Faza 1+2 |
| 5 | Źródło światła (ambient + directional lub point light) | ⏳ Faza 1 |
| 6 | Cieniowanie (Gouraud/Phong lub własne shadery) | ⏳ Faza 1 |
| 7 | Teksturowanie przynajmniej jednego obiektu | ⏳ Faza 4 |
| 8 | Animacja (ruch obiektu lub światła) | ⏳ Faza 3 |
| 9 | Komentarze + dokumentacja PDF | ⏳ Faza 6 |
| 10 | Filtrowanie obrazu | ⏳ Faza 6 |

---

## OPIS APLIKACJI (wymagania funkcjonalne)

Aplikacja do projektowania wnętrza mieszkania w środowisku 2D/3D.
Wszystkie odległości w metrach (możliwość wartości dziesiętnych).

### Interfejs
- Pusta przestrzeń robocza + **panel narzędzi po lewej stronie** (ImGui)
- Część funkcji zablokowana do momentu stworzenia obrysu mieszkania

### Tryby kamery
| Tryb | Opis |
|---|---|
| **Projektowy** | Widok z góry (top-down), tworzenie i edycja obrysu/ścian, brak swobodnego ruchu |
| **Obserwatora** | Swobodna kamera 3D bez ograniczeń (freelook) |
| **Eksploracyjny** | Symulacja chodzenia po mieszkaniu, brak latania, ruch ograniczony do wnętrza |

### Tworzenie obrysu mieszkania
- Zamknięty wielokąt 2D (rzut z góry)
- Użytkownik podaje: długość odcinka, kierunek (±), kąt względem poprzedniego
- Edycja i usuwanie odcinków podczas tworzenia
- Po zamknięciu: podanie wysokości + wskazanie ściany z drzwiami głównymi
- Punkt startowy = początek układu (0,0)
- Po zatwierdzeniu: odblokowują się pozostałe funkcje

### Ściany wewnętrzne
- Tryb projektowy (top-down)
- Punkt startowy (współrzędne względem 0,0 lub aktualnego punktu odniesienia)
- Grubość ściany + długość + kąt
- Można łączyć ściany (punkt końcowy jednej = startowy kolejnej)

### Drzwi
- Wybór ściany → położenie (początek/koniec od krawędzi) → wysokość
- Typ: tradycyjne (rozwierane) lub przesuwne
- Kierunek otwierania (dla tradycyjnych)
- **Animacja** otwierania/zamykania w trybie eksploracyjnym (po zbliżeniu)

### Okna
- Wybór ściany → wymiary (szerokość × wysokość) → położenie na ścianie
- Automatyczne wycięcie w geometrii ściany

### Przeszkody (elementy stałe)
- Import z pliku zewnętrznego LUB kreator
- Nie można przesuwać po umieszczeniu

### Meble
- Wbudowana biblioteka podstawowych mebli
- Import własnych modeli (OBJ)
- Kreator mebli
- Pozycjonowanie: ręczne (drag) lub wpisanie współrzędnych

### Kreator obiektów (osobne okno ImGui)
- Bryły: **sześcian, kula, graniastosłup trójkątny**
- Zmiana rozmiarów, łączenie brył w jeden model
- Po scaleniu: przeliczenie wymiarów, zapis jako jeden obiekt

### Tekstury i kolory
- Domyślna biblioteka tekstur dla ścian, podłóg, mebli
- Dodawanie własnych tekstur
- Zmiana materiałów wszystkich elementów

### Dodatkowe funkcje
- Zapis i wczytywanie projektów (JSON)
- Undo/Redo
- Edycja i usuwanie wszystkich elementów
- Walidacja danych (poprawność wymiarów)
- Opcjonalne wykrywanie kolizji
- Precyzyjne pozycjonowanie (współrzędne)
- Eksport projektu do pliku

---

## PLAN FAZ IMPLEMENTACJI

### FAZA 1 — Fundament: Renderer + Kamera + Shadery Phong
**Cel:** Działający silnik z kamerą 3D i oświetleniem. Spełnia pkt 3, 4, 5, 6 z PDF.

**Co powstanie:**
- Klasa `Shader` — ładowanie GLSL z pliku, kompilacja, linkowanie
- Shadery `phong.vert` + `phong.frag` — Phong z ambient + directional light
- Klasa `Camera` z 3 trybami (`TOP_DOWN`, `FREE`, `EXPLORE`)
- Siatka podłogi jako placeholder (VAO/VBO)
- Biblioteka **GLM** dodana do projektu (header-only, do `external/glm/`)

**Docelowa struktura po fazie 1:**
```
src/
├── main.cpp
├── core/
│   ├── Shader.h / Shader.cpp
│   └── Camera.h / Camera.cpp
shaders/
├── phong.vert
├── phong.frag
├── flat.vert
└── flat.frag
external/
└── glm/          ← dodać (header-only, bez kompilacji)
```

**CMakeLists.txt** — dodać do `target_include_directories`:
```cmake
external/glm
```

---

### FAZA 2 — Obrys mieszkania + Panel UI (ImGui)
**Cel:** Rysowanie wielokąta w top-down, wytłaczanie do 3D, panel narzędzi.

**Co powstanie:**
- Biblioteka **ImGui** w projekcie (`external/imgui/`)
- Klasa `Outline` — lista punktów 2D, logika zamykania kształtu
- Klasa `Wall` — generowanie geometrii ścian z krawędzi wielokąta
- Panel UI po lewej z narzędziami (ImGui)
- Dialog: wysokość mieszkania + wskazanie ściany z drzwiami

**Kluczowy algorytm wytłaczania krawędzi → ściana 3D:**
```
Krawędź 2D: A(x1,z1) → B(x2,z2), wysokość h
Ściana = quad z 4 wierzchołków:
  A_bot (x1, 0, z1)
  B_bot (x2, 0, z2)
  B_top (x2, h, z2)
  A_top (x1, h, z1)
```

**Docelowa struktura po fazie 2:**
```
src/
├── ui/
│   └── Panel.h / Panel.cpp
├── scene/
│   ├── Outline.h / Outline.cpp
│   └── Wall.h / Wall.cpp
external/
└── imgui/
    ├── imgui.h / imgui.cpp
    ├── imgui_impl_glfw.h/.cpp
    └── imgui_impl_opengl3.h/.cpp
```

---

### FAZA 3 — Ściany wewnętrzne + Drzwi + Okna
**Cel:** Wyposażanie wnętrza w elementy architektoniczne. Spełnia pkt 8 (animacja drzwi).

**Co powstanie:**
- `InnerWall` — prostopadłościan z punkt startowy + długość + kąt + grubość
- `Door` — wycięcie w ścianie + animowany quad (skrzydło), `float openAngle` interpolowany w czasie
- `Window` — wycięcie w ścianie na danej wysokości + przezroczysty quad
- Detekcja zbliżenia do drzwi w trybie EXPLORE → auto-animacja

---

### FAZA 4 — Meble + Teksturowanie
**Cel:** Biblioteka mebli, ładowanie tekstur. Spełnia pkt 2 (3 modele 3D) i pkt 7 (tekstury).

**Co powstanie:**
- Biblioteka **stb_image** (`external/stb/stb_image.h` — single header)
- Klasa `Texture` — `glGenTextures`, filtrowanie, mipmapping
- Klasa `Mesh` — VAO/VBO/EBO + `draw(Shader&)`
- Klasa `Model` — zestaw Mesh-y + transformacje
- Klasa `Furniture` — pozycja, rotacja, drag na scenie
- Shadery zaktualizowane o `uniform sampler2D texture1`
- Podstawowe meble: sofa, stół, szafa (prostopadłościany z teksturami)
- Drag & drop mebli w top-down + precyzyjne wpisywanie XZ

**Biblioteki do dodania:**
```
external/stb/stb_image.h    ← single header, bez kompilacji
```

**Foldery zasobów:**
```
assets/
├── textures/
│   ├── wall_white.png
│   ├── floor_wood.png
│   └── ...
└── models/
    ├── sofa.obj
    └── ...
```

---

### FAZA 5 — Kreator obiektów + Import OBJ
**Cel:** Tworzenie własnych modeli, import zewnętrzny.

**Co powstanie:**
- `ObjectCreator` — okno ImGui z listą brył (sześcian/kula/graniastosłup)
- Podgląd w czasie rzeczywistym tworzonego modelu
- `mergeToModel()` — scalanie brył w jeden `Model`
- Prosty OBJ loader (obsługa `v`, `vt`, `vn`, `f`)

---

### FAZA 6 — Zapis/Wczytywanie + Undo/Redo + Postprocessing
**Cel:** Polishing + domknięcie wszystkich wymagań PDF (pkt 9, 10).

**Co powstanie:**
- Serializacja do JSON — biblioteka **nlohmann/json** (`external/json/json.hpp`)
- `UndoRedo` — `std::stack<SceneState>`, Ctrl+Z / Ctrl+Y
- `PostProcessor` — FBO → quad → shader filtrujący (blur/sharpen) — **pkt 10 PDF**
- Shadery postprocessingu: `screen.vert` + `screen.frag`
- Dokumentacja PDF projektu

**Biblioteki do dodania:**
```
external/json/json.hpp    ← single header nlohmann/json
```

---

## MAPA WYMAGAŃ PDF → FAZY

| Pkt PDF | Faza | Co to realizuje |
|---|---|---|
| 1. Inicjalizacja OpenGL | ✅ GOTOWE | GLAD + GLFW + SDL3 |
| 2. Min. 3 modele 3D | Faza 4 | Ściany + meble + podłoga |
| 3. Rzutowanie perspektywiczne | Faza 1 | Klasa Camera, glm::perspective |
| 4. Interakcja mysz/klawiatura | Faza 1+2 | Camera + panel ImGui |
| 5. Oświetlenie ambient+directional | Faza 1 | Phong shader |
| 6. Cieniowanie Phong | Faza 1 | phong.vert + phong.frag |
| 7. Teksturowanie | Faza 4 | stb_image + Texture klasa |
| 8. Animacja | Faza 3 | Animacja drzwi |
| 9. Dokumentacja PDF | Faza 6 | Na końcu |
| 10. Filtrowanie obrazu | Faza 6 | FBO + screen.frag |

---

## BIBLIOTEKI DO POBRANIA W TRAKCIE PROJEKTU

| Biblioteka | Kiedy | Jak |
|---|---|---|
| **GLM** | Faza 1 | https://github.com/g-truc/glm → ZIP → wypakuj do `external/glm/` |
| **ImGui** | Faza 2 | https://github.com/ocornut/imgui → ZIP → skopiuj pliki do `external/imgui/` |
| **stb_image** | Faza 4 | https://github.com/nothings/stb → tylko `stb_image.h` → `external/stb/` |
| **nlohmann/json** | Faza 6 | https://github.com/nlohmann/json → tylko `json.hpp` → `external/json/` |

**Wszystkie powyższe to biblioteki header-only lub single-header — bez kompilacji, bez cmake find_package.**

---

## KOMENDY BUDOWANIA (do kopiowania)

```bash
# Pierwsze uruchomienie / po zmianie CMakeLists.txt:
cd C:\Users\studia\Desktop\proj_graf
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug

# Kolejne buildy (tylko rekompilacja):
cd C:\Users\studia\Desktop\proj_graf\build
mingw32-make

# Uruchomienie:
./OpenGLProject.exe
```

---

## STATUS FAZ

| Faza | Nazwa | Status |
|---|---|---|
| Faza 0 | Setup środowiska | ✅ GOTOWE |
| Faza 1 | Renderer + Kamera + Shadery Phong | ⏳ DO ZROBIENIA |
| Faza 2 | Obrys mieszkania + ImGui | ⏳ DO ZROBIENIA |
| Faza 3 | Ściany wew. + Drzwi + Okna | ⏳ DO ZROBIENIA |
| Faza 4 | Meble + Tekstury | ⏳ DO ZROBIENIA |
| Faza 5 | Kreator + Import OBJ | ⏳ DO ZROBIENIA |
| Faza 6 | Zapis/Undo/Postprocessing/Docs | ⏳ DO ZROBIENIA |

---

*Aby kontynuować projekt w nowym czacie: wklej ten plik i napisz "Jedziemy z fazą X".*
