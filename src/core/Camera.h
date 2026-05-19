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
    // ── Pozycja i orientacja ──────────────────────────────────────────
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    // ── Katy Eulera (dla trybu FREE i EXPLORE) ────────────────────────
    float yaw;    // obrot poziomy (lewo/prawo)
    float pitch;  // obrot pionowy (gora/dol)

    // ── Parametry ─────────────────────────────────────────────────────
    float moveSpeed;          // predkosc ruchu [m/s]
    float mouseSensitivity;   // czulosc myszy
    float fov;                // pole widzenia [stopnie] dla trybu FREE/EXPLORE
    float topDownZoom;        // zakres widoczny w trybie TOP_DOWN [metry]

    // ── Aktualny tryb ─────────────────────────────────────────────────
    CameraMode mode;

    // ── Konstruktor ───────────────────────────────────────────────────
    Camera();

    // ── Macierze ─────────────────────────────────────────────────────
    glm::mat4 getViewMatrix()                    const;
    glm::mat4 getProjectionMatrix(float aspect)  const;

    // ── Sterowanie ───────────────────────────────────────────────────
    void processKeyboard   (GLFWwindow* window, float deltaTime);
    void processMouseMove  (float xOffset, float yOffset);
    void processMouseScroll(float yOffset);

    // ── Zmiana trybu ─────────────────────────────────────────────────
    void setMode(CameraMode newMode);
    CameraMode getMode() const { return mode; }

private:
    void updateVectors();  // przelicza front/right/up z yaw i pitch
};