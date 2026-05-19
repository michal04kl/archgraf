#pragma once
#include "scene/Outline.h"
#include "core/Camera.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>

enum class AppState { DRAWING_OUTLINE, CONFIRM_OUTLINE, DESIGN_MODE };

struct PanelResult {
    bool  undoPoint=false,clearOutline=false,closeOutline=false;
    bool  confirmOutline=false,reopenOutline=false,placeNextPoint=false;
    float apartmentHeight=2.7f;
    int   doorWallIndex=0;
    float floorSize=30.0f;
    bool  floorSizeDirty=false;
    glm::vec2 nextPoint={0,0};
    bool  hasNextPoint=false;

    // IW
    bool  iwBeginSelectOrigin=false;
    bool  iwCancel=false;
    bool  iwPlacePoint=false;
    bool  iwCreateWalls=false;
    bool  iwUndo=false;
    float iw_moveX=0,iw_moveZ=0;
    float iw_moveAlong=0;
    float iw_thickness=0.15f;
    float iw_height=0.0f;
    int   deleteInnerWall=-1;

    // Drzwi
    bool  placeDoor=false,placePassage=false;
    int   door_wallIdx=0;
    bool  door_isInner=false;
    float door_offset=0.5f,door_width=0.9f,door_height=2.1f;
    bool  door_swing=true,door_openLeft=true,door_isPassage=false;
    int   deleteDoor=-1;

    // Okno
    bool  placeWindow=false;
    int   win_wallIdx=0;
    bool  win_isInner=false;
    float win_offset=0.1f,win_width=0.8f,win_height=1.0f,win_sill=0.9f;
    int   deleteWindow=-1;

    // Meble
    bool        startFurniturePlacement=false;  // wejdz w tryb stawiania
    bool        cancelFurniturePlacement=false;
    int         furnitureModelIdx=0;   // indeks w availableModels
    float       furnitureRotY=0.0f;
    float       furnitureScale=1.0f;
    int         deleteFurniture=-1;
};

enum class IWState { IDLE, SELECT_ORIGIN, PLACING };

struct IWMoveFlags {
    bool canLeft=false,canRight=false,canUp=false,canDown=false;
    bool canAlongLeft=false,canAlongRight=false;
    bool onWall=false;
    int  wallIdx=-1;
};

class Panel {
public:
    AppState state=AppState::DRAWING_OUTLINE;
    float apartmentHeight=2.7f;
    int   doorWallIndex=0;
    float snapGrid=0.25f,floorSize=30.0f;
    float inputLength=1.0f,inputAngle=0.0f;

    enum class DesignTool { NONE, INNER_WALL, DOOR, WINDOW, FURNITURE };
    DesignTool activeTool=DesignTool::NONE;

    // IW
    float iw_thickness=0.15f,iw_height=0.0f,iw_moveAmount=1.0f;

    // Drzwi
    int   door_wallIdx=0; bool door_isInner=false;
    float door_offset=0.5f,door_width=0.9f,door_height=2.1f;
    bool  door_swing=true,door_openLeft=true,door_isPassage=false;

    // Okno
    int   win_wallIdx=0; bool win_isInner=false;
    float win_offset=0.1f,win_width=0.8f,win_height=1.0f,win_sill=0.9f;

    // Meble
    int   furniture_modelIdx  = 0;
    float furniture_rotY      = 0.0f;
    float furniture_scale     = 1.0f;

    PanelResult render(
        const Outline& outline,
        CameraMode cameraMode,
        glm::vec2 snapCursor,
        int numInnerWalls, int numDoors, int numWindows,
        IWState iwState,
        glm::vec2 iwCursorPos,
        int iwPointsCount,
        const std::vector<float>& innerWallLengths,
        int iwOnWallIdx,
        float iwOnWallLen,
        const IWMoveFlags& moveFlags,
        // Meble
        const std::vector<std::string>& availableModels,
        int numFurnitureItems,
        bool furniturePlacementMode);

    glm::vec2 computeNextPoint(const Outline& outline) const;
    float     getAbsoluteAngle(const Outline& outline) const;
};