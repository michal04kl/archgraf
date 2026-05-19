#pragma once
#include <glm/glm.hpp>
#include "scene/Outline.h"

class RoomWindow {
public:
    // ── Dane logiczne (do cutoutow i renderowania) ────────────────────
    int   wallIndex   = 0;
    bool  isInnerWall = false;   // ← KLUCZOWE pole
    float offset      = 0.0f;
    float width       = 1.2f;
    float height      = 1.0f;
    float sill        = 0.9f;

    // ── Dane GL ───────────────────────────────────────────────────────
    unsigned int frameVAO=0, frameVBO=0;
    unsigned int glassVAO=0, glassVBO=0;
    int frameVertCount=0, glassVertCount=0;

    RoomWindow() = default;
    RoomWindow(const RoomWindow&) = delete;
    RoomWindow& operator=(const RoomWindow&) = delete;
    RoomWindow(RoomWindow&& o) noexcept;
    RoomWindow& operator=(RoomWindow&& o) noexcept;
    ~RoomWindow();

    void drawFrame() const;
    void drawGlass() const;
};