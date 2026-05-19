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
        // Kamera patrzy prostopadle w dol, os Z ekranu idzie w gore (Z ujemne)
        return glm::lookAt(
            position,
            glm::vec3(position.x, 0.0f, position.z),
            glm::vec3(0.0f, 0.0f, -1.0f)
        );
    }
    // FREE i EXPLORE: standardowy lookAt
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix(float aspect) const
{
    if (mode == CameraMode::TOP_DOWN) {
        // Rzutowanie ortograficzne — zachowuje proporcje
        float h = topDownZoom * 0.5f;
        float w = h * aspect;
        return glm::ortho(-w, w, -h, h, 0.1f, 200.0f);
    }
    // FREE i EXPLORE: rzutowanie perspektywiczne
    return glm::perspective(glm::radians(fov), aspect, 0.1f, 200.0f);
}

void Camera::processKeyboard(GLFWwindow* window, float deltaTime)
{
    float velocity = moveSpeed * deltaTime;

    if (mode == CameraMode::TOP_DOWN) {
        // Przesuniecia w planie XZ (widok z gory)
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) position.z -= velocity;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) position.z += velocity;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) position.x -= velocity;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) position.x += velocity;
        // Scroll zoomuje (patrz processMouseScroll)
    }
    else if (mode == CameraMode::FREE) {
        // Swobodny ruch we wszystkich kierunkach
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) position += front * velocity;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) position -= front * velocity;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) position -= right * velocity;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) position += right * velocity;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) position += worldUp * velocity;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) position -= worldUp * velocity;
    }
    else if (mode == CameraMode::EXPLORE) {
        // Chodzenie: ruch tylko w planie XZ (bez latania)
        glm::vec3 flatFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
        glm::vec3 flatRight = glm::normalize(glm::vec3(right.x, 0.0f, right.z));

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) position += flatFront * velocity;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) position -= flatFront * velocity;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) position -= flatRight * velocity;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) position += flatRight * velocity;

        // Zablokuj Y na wysokosci oczu (1.7m)
        position.y = 1.7f;
    }
}

void Camera::processMouseMove(float xOffset, float yOffset)
{
    // W trybie projektowym mysz nie obraca kamery
    if (mode == CameraMode::TOP_DOWN) return;

    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    yaw   += xOffset;
    pitch += yOffset;

    // Ogranicz pitch zeby nie "przewrocic" kamery
    pitch = std::clamp(pitch, -89.0f, 89.0f);

    updateVectors();
}

void Camera::processMouseScroll(float yOffset)
{
    if (mode == CameraMode::TOP_DOWN) {
        // Scroll = zoom: zmiana zakresu widocznego obszaru
        topDownZoom -= yOffset * 1.0f;
        topDownZoom  = std::clamp(topDownZoom, 3.0f, 80.0f);
    } else {
        // Scroll = zmiana FOV
        fov -= yOffset * 2.0f;
        fov  = std::clamp(fov, 10.0f, 90.0f);
    }
}

void Camera::setMode(CameraMode newMode)
{
    mode = newMode;

    if (newMode == CameraMode::TOP_DOWN) {
        // Reset pozycji do widoku z gory
        position = glm::vec3(0.0f, 10.0f, 0.0f);
    }
    else if (newMode == CameraMode::FREE) {
        // Ustaw kamera w punkcie startowym
        position = glm::vec3(0.0f, 3.0f, 5.0f);
        yaw      = -90.0f;
        pitch    = -20.0f;
        updateVectors();
    }
    else if (newMode == CameraMode::EXPLORE) {
        // Wejscie na poziom oczu
        position.y = 1.7f;
        pitch      = 0.0f;
        updateVectors();
    }
}

void Camera::updateVectors()
{
    // Oblicz nowy wektor front z katow Eulera
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    front = glm::normalize(newFront);
    right = glm::normalize(glm::cross(front, worldUp));
    up    = glm::normalize(glm::cross(right, front));
}