#include "Panel.h"
#include "scene/FurnitureLibrary.h"
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <cstring>

float Panel::getAbsoluteAngle(const Outline& outline) const
{
    float inputRad=glm::radians(inputAngle);
    if(outline.points.size()<2) return inputRad;
    const glm::vec2& prev=outline.points[outline.points.size()-2];
    const glm::vec2& last=outline.points.back();
    return std::atan2((last-prev).y,(last-prev).x)+inputRad;
}

glm::vec2 Panel::computeNextPoint(const Outline& outline) const
{
    if(outline.points.empty()||outline.closed) return {0,0};
    float a=getAbsoluteAngle(outline);
    const glm::vec2& last=outline.points.back();
    return {last.x+inputLength*std::cos(a),last.y+inputLength*std::sin(a)};
}

static void wallCombo(const char* id,
                      bool& isInner,int& wallIdx,
                      const Outline& outline,
                      const std::vector<float>& innerLens)
{
    int nOut=(int)outline.points.size();
    int nIn=(int)innerLens.size();
    char curLbl[64];
    if(!isInner)
        snprintf(curLbl,sizeof(curLbl),"Obrys %d (%.1fm)",wallIdx,outline.edgeLength(wallIdx));
    else {
        float l=(wallIdx<nIn)?innerLens[wallIdx]:0.f;
        snprintf(curLbl,sizeof(curLbl),"Wewn. %d (%.1fm)",wallIdx,l);
    }
    ImGui::SetNextItemWidth(-1);
    if(ImGui::BeginCombo(id,curLbl)){
        for(int i=0;i<nOut;++i){
            char buf[64]; snprintf(buf,sizeof(buf),"Obrys %d (%.1fm)",i,outline.edgeLength(i));
            bool sel=(!isInner&&wallIdx==i);
            if(ImGui::Selectable(buf,sel)){isInner=false;wallIdx=i;}
            if(sel) ImGui::SetItemDefaultFocus();
        }
        if(nIn>0){ImGui::Separator();ImGui::TextDisabled("  -- Sciany wewnetrzne --");}
        for(int i=0;i<nIn;++i){
            char buf[64]; snprintf(buf,sizeof(buf),"Wewn. %d (%.1fm)",i,innerLens[i]);
            bool sel=(isInner&&wallIdx==i);
            if(ImGui::Selectable(buf,sel)){isInner=true;wallIdx=i;}
            if(sel) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}

static bool dirButton(const char* label,bool canMove,float bw,float bh=28.0f)
{
    if(!canMove){
        ImGui::PushStyleColor(ImGuiCol_Button,       {0.18f,0.18f,0.18f,1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,{0.22f,0.22f,0.22f,1.0f});
        ImGui::PushStyleColor(ImGuiCol_Text,         {0.4f,0.4f,0.4f,1.0f});
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button,       {0.1f,0.3f,0.75f,1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,{0.2f,0.5f,0.9f,1.0f});
        ImGui::PushStyleColor(ImGuiCol_Text,         {1.0f,1.0f,1.0f,1.0f});
    }
    bool clicked=ImGui::Button(label,ImVec2(bw,bh));
    ImGui::PopStyleColor(3);
    return clicked&&canMove;
}

static bool alongButton(const char* label,bool canMove,float bw,float bh=28.0f)
{
    if(!canMove){
        ImGui::PushStyleColor(ImGuiCol_Button,       {0.18f,0.18f,0.18f,1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,{0.22f,0.22f,0.22f,1.0f});
        ImGui::PushStyleColor(ImGuiCol_Text,         {0.4f,0.4f,0.4f,1.0f});
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button,       {0.55f,0.32f,0.05f,1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,{0.75f,0.45f,0.1f,1.0f});
        ImGui::PushStyleColor(ImGuiCol_Text,         {1.0f,1.0f,1.0f,1.0f});
    }
    bool clicked=ImGui::Button(label,ImVec2(bw,bh));
    ImGui::PopStyleColor(3);
    return clicked&&canMove;
}

PanelResult Panel::render(
    const Outline& outline,
    CameraMode cameraMode,
    glm::vec2 snapCursor,
    int numInnerWalls,int numDoors,int numWindows,
    IWState iwState,
    glm::vec2 iwCursorPos,
    int iwPointsCount,
    const std::vector<float>& innerWallLengths,
    int iwOnWallIdx,
    float iwOnWallLen,
    const IWMoveFlags& moveFlags,
    const std::vector<std::string>& availableModels,
    const std::vector<std::string>& furnitureLabels,
    int selectedFurnitureIdx,
    glm::vec3 selectedFurniturePos,
    float selectedFurnitureRotY,
    bool furniturePlacementMode,
    bool builtinPlacementMode,
    bool furnitureMoveMode)
{
    PanelResult result;
    result.floorSize=floorSize;
    result.apartmentHeight=apartmentHeight;
    result.doorWallIndex=doorWallIndex;
    result.deleteInnerWall=-1;
    result.deleteDoor=-1;
    result.deleteWindow=-1;
    result.deleteFurniture=-1;
    result.iw_thickness=iw_thickness;
    result.iw_height=iw_height;
    result.iw_moveX=0;result.iw_moveZ=0;result.iw_moveAlong=0;
    result.furnitureModelIdx=furniture_modelIdx;
    result.furnitureRotY=furniture_rotY;
    result.furnitureScale=furniture_scale;
    result.startBuiltinPlacement=false;
    result.builtinFurnitureType=builtin_typeIdx;
    result.builtinRotY=builtin_rotY;
    result.builtinScale=builtin_scale;
    result.clickedFurnitureIdx  = -2;
    result.requestMoveFurniture = false;
    result.newFurnitureRotY     = -999.f;

    if(cameraMode!=CameraMode::TOP_DOWN){
        ImGui::SetNextWindowPos({10,10},ImGuiCond_Always);
        ImGui::SetNextWindowSize({260,80},ImGuiCond_Always);
        ImGuiWindowFlags hf=ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|
            ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoTitleBar|
            ImGuiWindowFlags_NoFocusOnAppearing;
        ImGui::Begin("##hint",nullptr,hf);
        ImGui::TextDisabled("[1] Wroc do trybu projektowego");
        if(state==AppState::DESIGN_MODE)
            ImGui::TextDisabled("[EXPLORE] Podejdz do drzwi aby otworzyc");
        ImGui::End();
        return result;
    }

    ImGui::SetNextWindowPos({0,0},ImGuiCond_Always);
    ImGui::SetNextWindowSize({300,ImGui::GetIO().DisplaySize.y},ImGuiCond_Always);
    ImGuiWindowFlags flags=ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|
        ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoFocusOnAppearing;
    ImGui::Begin("Interior Designer",nullptr,flags);

    ImGui::TextColored({0.4f,0.8f,1.0f,1.0f},"INTERIOR DESIGNER 3D");
    ImGui::Separator();
    ImGui::TextDisabled("Kamera: [1] Top  [2] Free  [3] Explore");
    ImGui::Spacing();

    // ═════ DRAWING_OUTLINE ═══════════════════════════════════════════
    if(state==AppState::DRAWING_OUTLINE){
        ImGui::TextColored({1.0f,0.85f,0.0f,1.0f},"RYSOWANIE OBRYSU");
        ImGui::Separator();

        float prevFloor=floorSize;
        ImGui::TextDisabled("Obszar roboczy:");
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderFloat("##floor",&floorSize,10,100,"%.0f x %.0f m");
        if(floorSize!=prevFloor){result.floorSize=floorSize;result.floorSizeDirty=true;}

        ImGui::TextDisabled("Siatka snap:");
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderFloat("##snap",&snapGrid,0.1f,2.0f,"%.2f m");
        ImGui::Spacing();ImGui::Separator();

        int nPts=(int)outline.points.size();
        if(nPts==0){
            ImGui::TextColored({0.8f,0.8f,0.4f,1.0f},"Krok 1: Punkt startowy");
            ImGui::TextWrapped("Kliknij LPM na siatce.");
        } else {
            ImGui::TextColored({0.8f,0.8f,0.4f,1.0f},"Nastepny odcinek:");
        }
        ImGui::Spacing();

        if(nPts>=1&&!outline.closed){
            ImGui::Text("Dlugosc (m):");
            ImGui::SetNextItemWidth(-1);
            ImGui::InputFloat("##len",&inputLength,0.25f,1.0f,"%.2f");
            if(inputLength<0.1f) inputLength=0.1f;
            ImGui::Spacing();
            if(nPts==1) ImGui::Text("Kierunek (deg od +X):");
            else{ ImGui::Text("Kat wzgledem poprzedniego:");
                  ImGui::TextDisabled("0=prosto 90=prawo -90=lewo"); }
            ImGui::SetNextItemWidth(-1);
            ImGui::InputFloat("##ang",&inputAngle,5.0f,15.0f,"%.1f deg");
            while(inputAngle>180.0f) inputAngle-=360.0f;
            while(inputAngle<=-180.0f) inputAngle+=360.0f;
            ImGui::Spacing();
            glm::vec2 next=computeNextPoint(outline);
            result.hasNextPoint=true;result.nextPoint=next;
            ImGui::TextColored({0.4f,1.0f,0.8f,1.0f},"Nastepny: (%.2f, %.2f)",next.x,next.y);
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Button,{0.2f,0.55f,0.2f,1.0f});
            if(ImGui::Button("Postaw punkt  (SPACJA)",ImVec2(-1,0))){
                result.placeNextPoint=true;result.nextPoint=next;
            }
            ImGui::PopStyleColor();
            ImGui::Spacing();ImGui::Separator();
        }

        ImGui::Text("Punkty: %d",nPts);
        if(nPts>=2) ImGui::Text("Obwod:  %.2f m",outline.perimeter());
        if(nPts>=2){
            ImGui::Spacing();ImGui::TextDisabled("Odcinki:");
            ImGui::BeginChild("segs",{0,70},true);
            for(int i=0;i+1<nPts;++i){
                glm::vec2 d=outline.points[i+1]-outline.points[i];
                ImGui::Text("  %d: %.2fm %.0fdeg",i,
                    glm::length(d),glm::degrees(std::atan2(d.y,d.x)));
            }
            ImGui::EndChild();
        }

        ImGui::Spacing();ImGui::Separator();
        ImGui::TextWrapped("LPM-postaw | SPACJA-dlugosc+kat | Z-cofnij | ENTER-zamknij");
        ImGui::Spacing();
        if(ImGui::Button("Cofnij punkt  (Z)",ImVec2(-1,0))) result.undoPoint=true;
        ImGui::BeginDisabled(!outline.canClose());
        ImGui::PushStyleColor(ImGuiCol_Button,{0.2f,0.6f,0.2f,1.0f});
        if(ImGui::Button("Zamknij obrys  (ENTER)",ImVec2(-1,0))) result.closeOutline=true;
        ImGui::PopStyleColor();
        ImGui::EndDisabled();
        if(ImGui::Button("Wyczysc wszystko",ImVec2(-1,0))) result.clearOutline=true;
    }
    // ═════ CONFIRM_OUTLINE ═══════════════════════════════════════════
    else if(state==AppState::CONFIRM_OUTLINE){
        ImGui::TextColored({0.4f,1.0f,0.4f,1.0f},"PARAMETRY MIESZKANIA");
        ImGui::Separator();
        ImGui::Text("Obwod:  %.2f m",outline.perimeter());
        ImGui::Text("Punkty: %d",(int)outline.points.size());
        ImGui::Spacing();
        ImGui::TextDisabled("Wysokosc pomieszczen:");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputFloat("##h",&apartmentHeight,0.1f,0.5f,"%.2f m");
        apartmentHeight=glm::clamp(apartmentHeight,1.5f,6.0f);
        ImGui::Spacing();ImGui::Separator();
        int nEdges=(int)outline.points.size();
        doorWallIndex=glm::clamp(doorWallIndex,0,nEdges-1);
        char lbl[64];
        snprintf(lbl,sizeof(lbl),"Sciana %d (%.1fm)",doorWallIndex,outline.edgeLength(doorWallIndex));
        ImGui::SetNextItemWidth(-1);
        if(ImGui::BeginCombo("##door",lbl)){
            for(int i=0;i<nEdges;++i){
                char buf[64]; snprintf(buf,sizeof(buf),"Sciana %d (%.1fm)",i,outline.edgeLength(i));
                bool sel=(doorWallIndex==i);
                if(ImGui::Selectable(buf,sel)) doorWallIndex=i;
                if(sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::Spacing();ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Button,{0.2f,0.5f,0.9f,1.0f});
        if(ImGui::Button("Zatwierdz i generuj sciany",ImVec2(-1,0))){
            result.confirmOutline=true;
            result.apartmentHeight=apartmentHeight;
            result.doorWallIndex=doorWallIndex;
        }
        ImGui::PopStyleColor();
        ImGui::Spacing();
        if(ImGui::Button("Wroc do edycji obrysu",ImVec2(-1,0)))
            result.reopenOutline=true;
    }
    // ═════ DESIGN_MODE ═══════════════════════════════════════════════
    else if(state==AppState::DESIGN_MODE){

        if(iwState==IWState::SELECT_ORIGIN||iwState==IWState::PLACING){
            if(iwState==IWState::SELECT_ORIGIN){
                ImGui::Separator();
                ImGui::TextColored({1.0f,0.9f,0.0f,1.0f},">> Krok 1: Kliknij punkt startowy");
                ImGui::TextWrapped("Kliknij LPM jeden z zoltych punktow.");
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Button,{0.7f,0.15f,0.15f,1.0f});
                if(ImGui::Button("Anuluj  (ESC)",ImVec2(-1,0))) result.iwCancel=true;
                ImGui::PopStyleColor();
                ImGui::Separator();
            } else {
                ImGui::Separator();
                ImGui::TextColored({0.2f,1.0f,0.4f,1.0f},">> Krok 2: Ustaw punkty sciany");
                ImGui::Text("Punktow: %d (min 2)",iwPointsCount);
                ImGui::TextColored({0.3f,0.9f,1.0f,1.0f},"Kursor: (%.2f, %.2f)",iwCursorPos.x,iwCursorPos.y);
                if(moveFlags.onWall&&moveFlags.wallIdx>=0)
                    ImGui::TextColored({1.0f,0.85f,0.2f,1.0f},"Na scianie outlinu #%d (%.2fm)",moveFlags.wallIdx,iwOnWallLen);
                else ImGui::TextDisabled("Wewnatrz outlinu");
                ImGui::Separator();

                ImGui::Text("Krok (m):");
                ImGui::SetNextItemWidth(-1);
                ImGui::InputFloat("##iwmove",&iw_moveAmount,0.25f,1.0f,"%.2f");
                if(iw_moveAmount<0.05f) iw_moveAmount=0.05f;
                ImGui::Spacing();

                float bw=80.0f,sp=4.0f;
                ImGui::TextDisabled("Ruch w przestrzeni XZ:");
                ImGui::Dummy(ImVec2(bw,28));ImGui::SameLine(0,sp);
                if(dirButton("Gora##G",moveFlags.canUp,bw)) result.iw_moveZ=-iw_moveAmount;
                if(dirButton("Lewo##L",moveFlags.canLeft,bw)) result.iw_moveX=-iw_moveAmount;
                ImGui::SameLine(0,sp);
                {
                    bool any=moveFlags.canLeft||moveFlags.canRight||moveFlags.canUp||moveFlags.canDown;
                    ImGui::PushStyleColor(ImGuiCol_Button,any?ImVec4(0.1f,0.1f,0.1f,1.f):ImVec4(0.5f,0.1f,0.1f,1.f));
                    ImGui::PushStyleColor(ImGuiCol_Text,any?ImVec4(0.5f,0.5f,0.5f,1.f):ImVec4(1.f,0.3f,0.3f,1.f));
                    ImGui::Button(any?"##cx":"!!##cx",ImVec2(bw,28));
                    ImGui::PopStyleColor(2);
                }
                ImGui::SameLine(0,sp);
                if(dirButton("Prawo##R",moveFlags.canRight,bw)) result.iw_moveX=+iw_moveAmount;
                ImGui::Dummy(ImVec2(bw,28));ImGui::SameLine(0,sp);
                if(dirButton("Dol##D",moveFlags.canDown,bw)) result.iw_moveZ=+iw_moveAmount;

                ImGui::Spacing();ImGui::Separator();
                if(moveFlags.onWall){
                    ImGui::TextDisabled("Wzdluz sciany #%d:",moveFlags.wallIdx);
                    float bw2=(ImGui::GetContentRegionAvail().x-4.0f)*0.5f;
                    if(alongButton("<< Lewo##WL",moveFlags.canAlongLeft,bw2)) result.iw_moveAlong=-iw_moveAmount;
                    ImGui::SameLine(0,4);
                    if(alongButton("Prawo >>##WP",moveFlags.canAlongRight,bw2)) result.iw_moveAlong=+iw_moveAmount;
                } else {
                    ImGui::TextDisabled("Wzdluz: dostepne gdy kursor NA scianie");
                }

                ImGui::Spacing();ImGui::Separator();ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Button,{0.1f,0.55f,0.1f,1.0f});
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,{0.2f,0.75f,0.2f,1.0f});
                if(ImGui::Button("Postaw punkt  (ENTER)",ImVec2(-1,0))) result.iwPlacePoint=true;
                ImGui::PopStyleColor(2);
                ImGui::BeginDisabled(iwPointsCount==0);
                if(ImGui::Button("Cofnij punkt  (Z)",ImVec2(-1,0))) result.iwUndo=true;
                ImGui::EndDisabled();
                ImGui::Spacing();
                ImGui::BeginDisabled(iwPointsCount<2);
                ImGui::PushStyleColor(ImGuiCol_Button,{0.55f,0.25f,0.05f,1.0f});
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,{0.75f,0.35f,0.1f,1.0f});
                if(ImGui::Button("Stworz sciane!  (SPACJA)",ImVec2(-1,0))) result.iwCreateWalls=true;
                ImGui::PopStyleColor(2);
                ImGui::EndDisabled();
                if(iwPointsCount<2) ImGui::TextColored({0.8f,0.4f,0.4f,1.0f},"  wymagane min 2 punkty");
                ImGui::Spacing();ImGui::Separator();ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Button,{0.7f,0.15f,0.15f,1.0f});
                if(ImGui::Button("Anuluj  (ESC)",ImVec2(-1,0))) result.iwCancel=true;
                ImGui::PopStyleColor();
                ImGui::Separator();
            }
            ImGui::End();
            result.apartmentHeight=apartmentHeight;
            result.doorWallIndex=doorWallIndex;
            return result;
        }

        ImGui::TextColored({0.4f,1.0f,0.4f,1.0f},"TRYB PROJEKTOWANIA");
        ImGui::Separator();
        ImGui::Text("Sciany: %d | Wys: %.1fm",(int)outline.points.size(),apartmentHeight);
        ImGui::Spacing();

        activeTool=DesignTool::NONE;

        if(ImGui::BeginTabBar("##dt")){

            // ── TAB: Sciana wewnetrzna ────────────────────────────────
            if(ImGui::BeginTabItem("Sciana wew.")){
                activeTool=DesignTool::INNER_WALL;
                ImGui::Spacing();
                ImGui::Text("Grubosc (m):");
                ImGui::SetNextItemWidth(-1);
                ImGui::InputFloat("##iwthick",&iw_thickness,0.05f,0.1f,"%.2f");
                iw_thickness=glm::clamp(iw_thickness,0.05f,1.0f);
                ImGui::Text("Wysokosc (m, 0=auto):");
                ImGui::SetNextItemWidth(-1);
                ImGui::InputFloat("##iwh",&iw_height,0.1f,0.5f,"%.2f");
                iw_height=glm::clamp(iw_height,0.0f,6.0f);
                if(iw_height<0.01f) ImGui::TextDisabled("  auto=%.1fm",apartmentHeight);
                ImGui::Spacing();ImGui::Separator();ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Button,{0.2f,0.5f,0.85f,1.0f});
                if(ImGui::Button("Stworz sciane wewnetrzna",ImVec2(-1,0)))
                    result.iwBeginSelectOrigin=true;
                ImGui::PopStyleColor();
                ImGui::Spacing();
                ImGui::TextWrapped("1. Kliknij punkt startowy\n2. Przesuwaj kursor\n3. Postaw min 2 punkty\n4. Stworz sciane");
                if(numInnerWalls>0){
                    ImGui::Spacing();ImGui::Separator();
                    ImGui::TextDisabled("Sciany wewnetrzne (%d):",numInnerWalls);
                    float lh=(float)std::min(numInnerWalls*22+6,90);
                    ImGui::BeginChild("##iwlist",{0,lh},true);
                    for(int i=0;i<numInnerWalls;++i){
                        ImGui::Text("Sciana #%d",i);ImGui::SameLine();
                        char btn[24];snprintf(btn,sizeof(btn),"X##iw%d",i);
                        ImGui::PushStyleColor(ImGuiCol_Button,{0.7f,0.15f,0.15f,1.0f});
                        if(ImGui::SmallButton(btn)) result.deleteInnerWall=i;
                        ImGui::PopStyleColor();
                    }
                    ImGui::EndChild();
                }
                ImGui::EndTabItem();
            }

            // ── TAB: Drzwi ────────────────────────────────────────────
            if(ImGui::BeginTabItem("Drzwi")){
                activeTool=DesignTool::DOOR;
                ImGui::Spacing();
                ImGui::Text("Sciana:");
                {int maxD=door_isInner?(int)innerWallLengths.size()-1:(int)outline.points.size()-1;
                 door_wallIdx=glm::clamp(door_wallIdx,0,glm::max(maxD,0));}
                wallCombo("##dwall",door_isInner,door_wallIdx,outline,innerWallLengths);
                ImGui::Spacing();
                float wl=door_isInner?(door_wallIdx<(int)innerWallLengths.size()?innerWallLengths[door_wallIdx]:0.f):outline.edgeLength(door_wallIdx);
                ImGui::TextDisabled("Dlugosc sciany: %.2fm",wl);
                ImGui::Text("Przesuniecie od startu (m):");
                ImGui::SetNextItemWidth(-1);
                ImGui::InputFloat("##doff",&door_offset,0.1f,0.5f,"%.2f");
                door_offset=glm::clamp(door_offset,0.0f,glm::max(0.0f,wl-0.1f));
                float maxDW=glm::max(0.1f,wl-door_offset);
                ImGui::Text("Szerokosc (m, max %.2f):",maxDW);
                ImGui::SetNextItemWidth(-1);
                ImGui::InputFloat("##dw",&door_width,0.1f,0.5f,"%.2f");
                door_width=glm::clamp(door_width,0.3f,glm::min(2.5f,maxDW));
                ImGui::Text("Wysokosc (m):");
                ImGui::SetNextItemWidth(-1);
                ImGui::InputFloat("##dh",&door_height,0.1f,0.5f,"%.2f");
                door_height=glm::clamp(door_height,1.5f,apartmentHeight);
                ImGui::Spacing();
                ImGui::Text("Typ otworu:");
                if(ImGui::RadioButton("Drzwi",!door_isPassage)) door_isPassage=false;
                ImGui::SameLine();
                if(ImGui::RadioButton("Przejscie",door_isPassage)) door_isPassage=true;
                if(!door_isPassage){
                    ImGui::Text("Skrzydlo:");ImGui::SameLine();
                    if(ImGui::RadioButton("Rozwierane",door_swing)) door_swing=true;
                    ImGui::SameLine();
                    if(ImGui::RadioButton("Przesuwne",!door_swing)) door_swing=false;
                    if(door_swing){
                        ImGui::Text("Zawias:");ImGui::SameLine();
                        if(ImGui::RadioButton("Lewy",door_openLeft)) door_openLeft=true;
                        ImGui::SameLine();
                        if(ImGui::RadioButton("Prawy",!door_openLeft)) door_openLeft=false;
                    }
                } else ImGui::TextDisabled("  (bez skrzydla)");
                ImGui::Spacing();
                bool valid=(door_offset+door_width)<=wl;
                if(!valid) ImGui::TextColored({1,0.3f,0.3f,1},"! Wykracza (%.2fm)!",wl);
                if(valid) ImGui::TextColored({1.0f,0.85f,0.3f,1.0f},"Pozycja: %.2f-%.2fm",door_offset,door_offset+door_width);
                ImGui::BeginDisabled(!valid);
                ImGui::PushStyleColor(ImGuiCol_Button,{0.2f,0.4f,0.8f,1.0f});
                if(ImGui::Button("Dodaj",ImVec2(-1,0))){
                    result.placeDoor=!door_isPassage;
                    result.placePassage=door_isPassage;
                    result.door_wallIdx=door_wallIdx;
                    result.door_isInner=door_isInner;
                    result.door_offset=door_offset;
                    result.door_width=door_width;
                    result.door_height=door_height;
                    result.door_swing=door_swing;
                    result.door_openLeft=door_openLeft;
                    result.door_isPassage=door_isPassage;
                }
                ImGui::PopStyleColor();
                ImGui::EndDisabled();
                if(numDoors>0){
                    ImGui::Separator();
                    ImGui::TextDisabled("Otwory (%d):",numDoors);
                    float lh=(float)std::min(numDoors*22+6,90);
                    ImGui::BeginChild("##dlist",{0,lh},true);
                    for(int i=0;i<numDoors;++i){
                        ImGui::Text("Otw.#%d",i);ImGui::SameLine();
                        char btn[24];snprintf(btn,sizeof(btn),"X##d%d",i);
                        ImGui::PushStyleColor(ImGuiCol_Button,{0.7f,0.15f,0.15f,1.0f});
                        if(ImGui::SmallButton(btn)) result.deleteDoor=i;
                        ImGui::PopStyleColor();
                    }
                    ImGui::EndChild();
                }
                ImGui::EndTabItem();
            }

            // ── TAB: Okno ─────────────────────────────────────────────
            if(ImGui::BeginTabItem("Okno")){
                activeTool=DesignTool::WINDOW;
                ImGui::Spacing();
                ImGui::Text("Sciana:");
                {int maxW=win_isInner?(int)innerWallLengths.size()-1:(int)outline.points.size()-1;
                 win_wallIdx=glm::clamp(win_wallIdx,0,glm::max(maxW,0));}
                wallCombo("##wwall",win_isInner,win_wallIdx,outline,innerWallLengths);
                ImGui::Spacing();
                float wl=win_isInner?(win_wallIdx<(int)innerWallLengths.size()?innerWallLengths[win_wallIdx]:0.f):outline.edgeLength(win_wallIdx);
                ImGui::TextDisabled("Dlugosc sciany: %.2fm",wl);
                ImGui::Text("Przesuniecie od startu (m):");
                ImGui::SetNextItemWidth(-1);
                ImGui::InputFloat("##woff",&win_offset,0.1f,0.5f,"%.2f");
                win_offset=glm::clamp(win_offset,0.0f,glm::max(0.0f,wl-0.1f));
                float maxWW=glm::max(0.1f,wl-win_offset);
                ImGui::Text("Szerokosc (m, max %.2f):",maxWW);
                ImGui::SetNextItemWidth(-1);
                ImGui::InputFloat("##ww",&win_width,0.1f,0.5f,"%.2f");
                win_width=glm::clamp(win_width,0.1f,glm::min(3.0f,maxWW));
                ImGui::Text("Wysokosc (m):");
                ImGui::SetNextItemWidth(-1);
                ImGui::InputFloat("##wh",&win_height,0.1f,0.5f,"%.2f");
                win_height=glm::clamp(win_height,0.2f,apartmentHeight-0.2f);
                ImGui::Text("Parapet (m):");
                ImGui::SetNextItemWidth(-1);
                ImGui::InputFloat("##wsill",&win_sill,0.1f,0.5f,"%.2f");
                win_sill=glm::clamp(win_sill,0.0f,apartmentHeight-win_height);
                ImGui::Spacing();
                bool valid=(win_offset+win_width)<=wl;
                if(!valid) ImGui::TextColored({1,0.3f,0.3f,1},"! Wykracza (%.2fm)!",wl);
                if(valid) ImGui::TextColored({0.3f,0.9f,1.0f,1.0f},"Pozycja: %.2f-%.2fm, parapet %.2fm",win_offset,win_offset+win_width,win_sill);
                ImGui::BeginDisabled(!valid);
                ImGui::PushStyleColor(ImGuiCol_Button,{0.2f,0.55f,0.7f,1.0f});
                if(ImGui::Button("Dodaj okno",ImVec2(-1,0))){
                    result.placeWindow=true;
                    result.win_wallIdx=win_wallIdx;
                    result.win_isInner=win_isInner;
                    result.win_offset=win_offset;
                    result.win_width=win_width;
                    result.win_height=win_height;
                    result.win_sill=win_sill;
                }
                ImGui::PopStyleColor();
                ImGui::EndDisabled();
                if(numWindows>0){
                    ImGui::Separator();
                    ImGui::TextDisabled("Okna (%d):",numWindows);
                    float lh=(float)std::min(numWindows*22+6,90);
                    ImGui::BeginChild("##wlist",{0,lh},true);
                    for(int i=0;i<numWindows;++i){
                        ImGui::Text("Okno#%d",i);ImGui::SameLine();
                        char btn[24];snprintf(btn,sizeof(btn),"X##w%d",i);
                        ImGui::PushStyleColor(ImGuiCol_Button,{0.7f,0.15f,0.15f,1.0f});
                        if(ImGui::SmallButton(btn)) result.deleteWindow=i;
                        ImGui::PopStyleColor();
                    }
                    ImGui::EndChild();
                }
                ImGui::EndTabItem();
            }

            // ── TAB: Meble ────────────────────────────────────────────────────
if(ImGui::BeginTabItem("Meble")){
    activeTool=DesignTool::FURNITURE;
    ImGui::Spacing();

    // ─── BIBLIOTEKA WBUDOWANA ─────────────────────────────────────
    ImGui::TextColored({0.4f,1.0f,0.8f,1.0f},"Meble wbudowane:");
    ImGui::Spacing();

    static const char* cats[]={"Salon / Jadalnia","Sypialnia","Kuchnia","Lazienka"};
    for(const char* cat:cats){
        if(ImGui::CollapsingHeader(cat)){
            ImGui::Indent(8.0f);
            for(int i=0;i<(int)FurnitureType::COUNT;++i){
                FurnitureType ft=(FurnitureType)i;
                if(strcmp(furnitureCategory(ft),cat)!=0) continue;
                bool sel=(builtin_typeIdx==i);
                if(ImGui::Selectable(furnitureName(ft),sel))
                    builtin_typeIdx=i;
            }
            ImGui::Unindent(8.0f);
        }
    }

    ImGui::Spacing();
    ImGui::Text("Obrot Y:"); ImGui::SameLine();
    ImGui::SetNextItemWidth(120);
    ImGui::SliderFloat("##brotY",&builtin_rotY,0,360,"%.0f deg");
    ImGui::Spacing();

    if(!builtinPlacementMode){
        ImGui::PushStyleColor(ImGuiCol_Button,{0.15f,0.55f,0.25f,1.0f});
        if(ImGui::Button("Postaw wbudowany",ImVec2(-1,0))){
            result.startBuiltinPlacement=true;
            result.builtinFurnitureType=builtin_typeIdx;
            result.builtinRotY=builtin_rotY;
            result.builtinScale=1.0f;
        }
        ImGui::PopStyleColor();
        ImGui::TextDisabled("Potem kliknij LPM w widoku");
    } else {
        ImGui::TextColored({0.2f,1.0f,0.5f,1.0f},
            ">> LPM: %s",furnitureName((FurnitureType)builtin_typeIdx));
        ImGui::TextDisabled("R = obrot 45 deg | ESC = anuluj");
        ImGui::PushStyleColor(ImGuiCol_Button,{0.7f,0.15f,0.15f,1.0f});
        if(ImGui::Button("Anuluj (ESC)##b",ImVec2(-1,0)))
            result.cancelFurniturePlacement=true;
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();ImGui::Separator();ImGui::Spacing();

    // ─── MODELE OBJ ───────────────────────────────────────────────
    ImGui::TextColored({1.0f,0.85f,0.4f,1.0f},"Modele OBJ:");
    ImGui::Spacing();

    if(availableModels.empty()){
        ImGui::TextColored({0.7f,0.7f,0.7f,1.0f},"Brak plikow .obj");
        ImGui::TextWrapped("Umiec .obj w: assets/models/");
    } else {
        furniture_modelIdx=glm::clamp(furniture_modelIdx,0,(int)availableModels.size()-1);
        float listH=(float)glm::min((int)availableModels.size()*22+6,110);
        ImGui::BeginChild("##modlist",{-1,listH},true);
        for(int i=0;i<(int)availableModels.size();++i){
            bool sel=(furniture_modelIdx==i);
            if(ImGui::Selectable(availableModels[i].c_str(),sel))
                furniture_modelIdx=i;
        }
        ImGui::EndChild();
        ImGui::Text("Obrot Y:"); ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        ImGui::SliderFloat("##frotY",&furniture_rotY,-180.0f,180.0f,"%.0f");
        ImGui::Text("Skala:"); ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        ImGui::SliderFloat("##fscale",&furniture_scale,0.05f,5.0f,"%.2f");
        ImGui::Spacing();
        if(!furniturePlacementMode){
            ImGui::PushStyleColor(ImGuiCol_Button,{0.2f,0.55f,0.2f,1.0f});
            if(ImGui::Button("Postaw OBJ",ImVec2(-1,0))){
                result.startFurniturePlacement=true;
                result.furnitureModelIdx=furniture_modelIdx;
                result.furnitureRotY=furniture_rotY;
                result.furnitureScale=furniture_scale;
            }
            ImGui::PopStyleColor();
            ImGui::TextDisabled("Potem kliknij LPM w widoku");
        } else {
            ImGui::TextColored({0.2f,1.0f,0.4f,1.0f},
                ">> LPM: %s",availableModels[furniture_modelIdx].c_str());
            ImGui::PushStyleColor(ImGuiCol_Button,{0.7f,0.15f,0.15f,1.0f});
            if(ImGui::Button("Anuluj (ESC)##o",ImVec2(-1,0)))
                result.cancelFurniturePlacement=true;
            ImGui::PopStyleColor();
        }
    }

    ImGui::Spacing();ImGui::Separator();ImGui::Spacing();

    // ─── POSTAWIONE MEBLE ─────────────────────────────────────────
    int nF=(int)furnitureLabels.size();
    if(nF>0){
        ImGui::TextDisabled("Postawione (%d) — kliknij aby zaznaczyc:",nF);
        float lh=(float)std::min(nF*22+6,120);
        ImGui::BeginChild("##flist",{0,lh},true);
        for(int i=0;i<nF;++i){
            bool isSel=(i==selectedFurnitureIdx);
            char buf[128];
            snprintf(buf,sizeof(buf),"[%d] %s###fi%d",i,furnitureLabels[i].c_str(),i);
            if(ImGui::Selectable(buf,isSel))
                result.clickedFurnitureIdx=i;
            ImGui::SameLine();
            char delbtn[24];snprintf(delbtn,sizeof(delbtn),"X##fd%d",i);
            ImGui::PushStyleColor(ImGuiCol_Button,{0.7f,0.15f,0.15f,1.0f});
            if(ImGui::SmallButton(delbtn)) result.deleteFurniture=i;
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();

        // Panel zaznaczonego mebla
        if(selectedFurnitureIdx>=0&&selectedFurnitureIdx<nF){
            ImGui::Separator();
            ImGui::TextColored({0.3f,1.0f,0.6f,1.0f},
                "Zaznaczony: %s",furnitureLabels[selectedFurnitureIdx].c_str());
            ImGui::Text("Pozycja: (%.2f, %.2f)",selectedFurniturePos.x,selectedFurniturePos.z);
            float rotBuf=selectedFurnitureRotY;
            ImGui::Text("Obrot Y:");ImGui::SameLine();
            ImGui::SetNextItemWidth(110);
            if(ImGui::SliderFloat("##serot",&rotBuf,0,360,"%.0f deg"))
                result.newFurnitureRotY=rotBuf;
            ImGui::Spacing();
            if(!furnitureMoveMode){
                ImGui::PushStyleColor(ImGuiCol_Button,{0.2f,0.4f,0.8f,1.0f});
                if(ImGui::Button("Przesuń (kliknij nowe miejsce)",ImVec2(-1,0)))
                    result.requestMoveFurniture=true;
                ImGui::PopStyleColor();
                if(ImGui::Button("Odznacz",ImVec2(-1,0)))
                    result.clickedFurnitureIdx=-1;
            } else {
                ImGui::TextColored({0.2f,1.0f,0.5f,1.0f},">> Kliknij nowe miejsce...");
                ImGui::TextDisabled("ESC = anuluj");
                ImGui::PushStyleColor(ImGuiCol_Button,{0.7f,0.15f,0.15f,1.0f});
                if(ImGui::Button("Anuluj przesuwanie##cncmv",ImVec2(-1,0)))
                    result.cancelFurniturePlacement=true;
                ImGui::PopStyleColor();
            }
        }
    }

    ImGui::EndTabItem();
}

            ImGui::EndTabBar();
        }

        ImGui::Spacing();ImGui::Separator();ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Button,{0.7f,0.4f,0.1f,1.0f});
        if(ImGui::Button("Edytuj obrys",ImVec2(-1,0))) result.reopenOutline=true;
        ImGui::PopStyleColor();
    }

    ImGui::End();
    result.apartmentHeight=apartmentHeight;
    result.doorWallIndex=doorWallIndex;
    return result;
}