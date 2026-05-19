#pragma once

#include <vector>
#include <glm/glm.hpp>

// Przechowuje obrys mieszkania jako liste punktow 2D (wspolrzedne X, Z).
// Klasa jest czysto danych - bez logiki OpenGL.
class Outline {
public:
    std::vector<glm::vec2> points;
    bool closed = false;

    // Odleglosc snappingu do zamkniecia (metry)
    static constexpr float CLOSE_DIST = 0.5f;

    // Dodaje punkt — tylko jesli obrys jest otwarty
    void addPoint(glm::vec2 p) {
        if (!closed) points.push_back(p);
    }

    // Usuwa ostatni punkt
    void removeLast() {
        if (!points.empty() && !closed) points.pop_back();
    }

    // Resetuje caly obrys
    void clear() {
        points.clear();
        closed = false;
    }

    // Zamknij obrys (min 3 punkty). Zwraca true jesli sie udalo.
    bool tryClose() {
        if (points.size() >= 3) { closed = true; return true; }
        return false;
    }

    // Otwiera zamkniety obrys do edycji (zachowuje punkty)
    void reopen() {
        if (!closed) return;
        closed = false;
        // Usun duplikat punktu startowego jesli istnieje
        if (points.size() >= 2 &&
            glm::length(points.back() - points[0]) < 0.001f)
        {
            points.pop_back();
        }
    }

    // Czy punkt p jest blisko punktu startowego
    bool isNearStart(glm::vec2 p) const {
        return !points.empty() && glm::length(p - points[0]) < CLOSE_DIST;
    }

    bool canClose() const { return points.size() >= 3 && !closed; }

    // Srodek ciezkosci obrysu
    glm::vec2 centroid() const {
        glm::vec2 c{};
        for (auto& p : points) c += p;
        return points.empty() ? c : c / (float)points.size();
    }

    // Obwod obrysu w metrach
    float perimeter() const {
        float total = 0.0f;
        for (size_t i = 1; i < points.size(); ++i)
            total += glm::length(points[i] - points[i-1]);
        if (closed && points.size() >= 2)
            total += glm::length(points[0] - points.back());
        return total;
    }

    // Dlugosc odcinka o danym indeksie
    float edgeLength(int i) const {
        if ((int)points.size() < 2) return 0.0f;
        int j = (i + 1) % (int)points.size();
        return glm::length(points[j] - points[i]);
    }
};