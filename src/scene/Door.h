#pragma once
#include <glm/glm.hpp>

class Outline;

class Door {
public:
    enum class Type { SWING, SLIDING };

    // Dane geometryczne sciany (wspolne dla outline i inner walls)
    glm::vec2 wStart;
    glm::vec2 wDir;
    glm::vec2 wInward;
    glm::vec2 hingeXZ;
    glm::vec3 worldCenter;

    int   wallIndex     = -1;  // -1 = sciana wewnetrzna
    bool  isInnerWall   = false;
    float offset        = 0.0f;
    float width         = 0.9f;
    float height        = 2.1f;
    Type  type          = Type::SWING;
    bool  openLeft      = true;
    bool  isPassage     = false;  // przejscie bez skrzydla

    float openAngle    = 0.0f;
    float targetAngle  = 0.0f;
    static constexpr float OPEN_SPEED   = 200.0f;
    static constexpr float TRIGGER_DIST = 2.0f;

    unsigned int frameVAO=0, frameVBO=0;
    unsigned int panelVAO=0, panelVBO=0;
    int frameVertCount=0;

    Door() = default;

    // Konstruktor dla scian obrysu
    Door(int wallIdx, float off, float w, float h,
         Type t, bool openLeft, bool passage,
         const Outline& outline, float aptHeight);

    // Konstruktor dla scian wewnetrznych
    Door(glm::vec2 wallA, glm::vec2 wallB, glm::vec2 inward,
         float off, float w, float h,
         Type t, bool openLeft, bool passage, float aptHeight);

    Door(const Door&) = delete;
    Door& operator=(const Door&) = delete;
    Door(Door&& o) noexcept;
    Door& operator=(Door&& o) noexcept;
    ~Door();

    void buildFrame();
    void initPanel();
    void updatePanel();
    void update(float dt, glm::vec3 playerPos);
    void drawFrame() const;
    void drawPanel() const;

private:
    void initFromWall(glm::vec2 A, glm::vec2 B, glm::vec2 inward,
                      float off, float w, float h,
                      Type t, bool oLeft, bool passage, float aptH);
};