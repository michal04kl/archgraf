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
#include "scene/InnerWall.h"
#include "scene/Door.h"
#include "scene/RoomWindow.h"
#include "scene/Furniture.h"
#include "ui/Panel.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

const int SCR_WIDTH=1280,SCR_HEIGHT=720;

Camera camera;
float deltaTime=0,lastFrame=0;
float lastMouseX=SCR_WIDTH/2.0f,lastMouseY=SCR_HEIGHT/2.0f;
bool  firstMouse=true,mouseCapured=false;
double cursorScreenX=SCR_WIDTH/2.0,cursorScreenY=SCR_HEIGHT/2.0;
bool pendingLeftClick=false,pendingRightClick=false;
bool pendingSpace=false,pendingEnter=false;

Outline outline;
std::vector<Wall>       walls;
std::vector<InnerWall>  innerWalls;
std::vector<Door>       doors;
std::vector<RoomWindow> roomWindows;
FurnitureManager        furnitureManager;
Panel panel;
float apartmentHeight=2.7f;

IWState        iwState=IWState::IDLE;
glm::vec2      iwCursor={0,0};
std::vector<glm::vec2> iwPoints;
bool wallsDirty=false;

bool              furniturePlacementMode=false;
int               furniturePlacingModelIdx=0;
float             furniturePlacingRotY=0.0f;
float             furniturePlacingScale=1.0f;

bool          builtinPlacementMode=false;
FurnitureType builtinPlacingType=FurnitureType::CHAIR;
float         builtinPlacingRotY=0.0f;
float         builtinPlacingScale=1.0f;

// Faza 5
int   selectedFurnitureIdx = -1;
bool  furnitureMoveMode    = false;

std::vector<std::string> availableModels;

void scanModels() {
    availableModels.clear();
    if (fs::exists("assets/models")) {
        for (auto& entry : fs::directory_iterator("assets/models")) {
            if (entry.path().extension() == ".obj")
                availableModels.push_back(entry.path().filename().string());
        }
        std::sort(availableModels.begin(), availableModels.end());
    }
}

std::string findTexture(const std::string& modelFile) {
    std::string stem = modelFile.substr(0, modelFile.rfind('.'));
    for (auto& ext : {".png",".jpg",".jpeg",".bmp"}) {
        std::string tname = stem + ext;
        if (fs::exists("assets/textures/" + tname))
            return tname;
    }
    return "";
}

std::vector<WallCutout> getCutoutsForOutlineWall(int wallIdx)
{
    std::vector<WallCutout> cuts;
    for(const auto& d:doors)
        if(!d.isInnerWall&&d.wallIndex==wallIdx)
            cuts.push_back({d.offset,d.offset+d.width,0.0f,d.height});
    for(const auto& rw:roomWindows)
        if(!rw.isInnerWall&&rw.wallIndex==wallIdx)
            cuts.push_back({rw.offset,rw.offset+rw.width,rw.sill,rw.sill+rw.height});
    return cuts;
}

std::vector<WallCutout> getCutoutsForInnerWall(int iwIdx)
{
    std::vector<WallCutout> cuts;
    for(const auto& d:doors)
        if(d.isInnerWall&&d.wallIndex==iwIdx)
            cuts.push_back({d.offset,d.offset+d.width,0.0f,d.height});
    for(const auto& rw:roomWindows)
        if(rw.isInnerWall&&rw.wallIndex==iwIdx)
            cuts.push_back({rw.offset,rw.offset+rw.width,rw.sill,rw.sill+rw.height});
    return cuts;
}

void rebuildAllWalls()
{
    glm::vec2 c=outline.centroid();
    for(int i=0;i<(int)walls.size();++i)
        walls[i].buildWithCutouts(c,getCutoutsForOutlineWall(i));
    for(int i=0;i<(int)innerWalls.size();++i)
        innerWalls[i].buildWithCutouts(getCutoutsForInnerWall(i));
    wallsDirty=false;
}

std::vector<glm::vec2> getSnapPoints()
{
    std::vector<glm::vec2> pts;
    for(auto& p:outline.points) pts.push_back(p);
    for(auto& iw:innerWalls){
        float rad=glm::radians(iw.angle);
        glm::vec2 dir={std::cos(rad),std::sin(rad)};
        pts.push_back(iw.startXZ);
        pts.push_back(iw.startXZ+dir*iw.length);
    }
    return pts;
}

glm::vec2 findNearest(glm::vec2 p,const std::vector<glm::vec2>& pts,float& outDist)
{
    outDist=1e9f; glm::vec2 best=p;
    for(auto& s:pts){float d=glm::length(p-s);if(d<outDist){outDist=d;best=s;}}
    return best;
}

bool pointInPolygon(glm::vec2 p,const std::vector<glm::vec2>& poly)
{
    if(poly.size()<3) return false;
    bool inside=false; int n=(int)poly.size();
    for(int i=0,j=n-1;i<n;j=i++){
        float xi=poly[i].x,yi=poly[i].y,xj=poly[j].x,yj=poly[j].y;
        if(((yi>p.y)!=(yj>p.y))&&(p.x<(xj-xi)*(p.y-yi)/(yj-yi)+xi)) inside=!inside;
    }
    return inside;
}

glm::vec2 segPushout(glm::vec2 p,glm::vec2 A,glm::vec2 B,float r)
{
    glm::vec2 AB=B-A; float len=glm::length(AB);
    if(len<0.001f) return {0,0};
    float t=glm::clamp(glm::dot(p-A,AB/len),0.0f,len);
    glm::vec2 cl=A+(AB/len)*t,d=p-cl; float dist=glm::length(d);
    if(dist<r&&dist>0.0001f) return glm::normalize(d)*(r-dist);
    return {0,0};
}

void getInnerWallCollisionSegs(int iwIdx,
    std::vector<std::pair<glm::vec2,glm::vec2>>& outSegs)
{
    const InnerWall& iw=innerWalls[iwIdx];
    float rad=glm::radians(iw.angle);
    glm::vec2 dir={std::cos(rad),std::sin(rad)};
    glm::vec2 perp={-std::sin(rad),std::cos(rad)};
    float ht=iw.thickness*0.5f;
    glm::vec2 SL=iw.startXZ+perp*ht;
    glm::vec2 SR=iw.startXZ-perp*ht;
    glm::vec2 EL=iw.startXZ+dir*iw.length+perp*ht;
    glm::vec2 ER=iw.startXZ+dir*iw.length-perp*ht;
    auto cuts=getCutoutsForInnerWall(iwIdx);
    std::vector<WallCutout> walkCuts;
    for(const auto& c:cuts)
        if(c.yBottom<0.05f&&c.yTop>iw.height*0.75f)
            walkCuts.push_back(c);
    std::sort(walkCuts.begin(),walkCuts.end(),
        [](const WallCutout& a,const WallCutout& b){ return a.offsetStart<b.offsetStart; });
    float cur=0.0f;
    for(const auto& c:walkCuts){
        float cs=glm::clamp(c.offsetStart,0.0f,iw.length);
        float ce=glm::clamp(c.offsetEnd,  0.0f,iw.length);
        if(cur<cs-0.001f) outSegs.push_back({SL+dir*cur,SL+dir*cs});
        cur=ce;
    }
    if(cur<iw.length-0.001f) outSegs.push_back({SL+dir*cur,EL});
    cur=0.0f;
    for(const auto& c:walkCuts){
        float cs=glm::clamp(iw.length-c.offsetEnd,  0.0f,iw.length);
        float ce=glm::clamp(iw.length-c.offsetStart,0.0f,iw.length);
        if(cur<cs-0.001f) outSegs.push_back({ER+(-dir)*cur,ER+(-dir)*cs});
        cur=ce;
    }
    if(cur<iw.length-0.001f) outSegs.push_back({ER+(-dir)*cur,SR});
    outSegs.push_back({SR,SL});
    outSegs.push_back({EL,ER});
}

glm::vec2 resolveCollisions(glm::vec2 pos)
{
    int n=(int)outline.points.size(); if(n<3) return pos;
    for(int it=0;it<3;++it)
        for(int i=0;i<n;++i)
            pos+=segPushout(pos,outline.points[i],outline.points[(i+1)%n],0.25f);
    for(int iwIdx=0;iwIdx<(int)innerWalls.size();++iwIdx){
        std::vector<std::pair<glm::vec2,glm::vec2>> segs;
        getInnerWallCollisionSegs(iwIdx,segs);
        for(int it=0;it<2;++it)
            for(auto& [A,B]:segs)
                pos+=segPushout(pos,A,B,0.25f);
    }
    return pos;
}

static float raySegIntersect(glm::vec2 O,glm::vec2 D,glm::vec2 A,glm::vec2 B)
{
    glm::vec2 E=B-A;
    float denom=D.x*E.y-D.y*E.x;
    if(std::abs(denom)<1e-7f) return -1.f;
    glm::vec2 diff=A-O;
    float t=(diff.x*E.y-diff.y*E.x)/denom;
    float s=(diff.x*D.y-diff.y*D.x)/denom;
    if(t>=0&&s>=0&&s<=1) return t;
    return -1.f;
}

glm::vec2 moveIwCursor(glm::vec2 cur,float dx,float dz)
{
    if(dx==0&&dz==0) return cur;
    glm::vec2 move={dx,dz};
    float dist=glm::length(move);
    glm::vec2 dir=move/dist;
    glm::vec2 next=cur+move;
    if(pointInPolygon(next,outline.points)) return next;
    int n=(int)outline.points.size();
    float minT=dist+1.f;
    for(int i=0;i<n;i++){
        float t=raySegIntersect(cur,dir,outline.points[i],outline.points[(i+1)%n]);
        if(t>=0&&t<minT) minT=t;
    }
    if(minT<=dist){
        float safeDist=glm::max(0.0f,minT-0.001f);
        glm::vec2 onWall=cur+dir*safeDist;
        glm::vec2 cen=outline.centroid();
        glm::vec2 inside=onWall+glm::normalize(cen-onWall)*0.002f;
        if(pointInPolygon(inside,outline.points)) return inside;
        if(pointInPolygon(onWall,outline.points)) return onWall;
        return cur;
    }
    glm::vec2 onlyX={cur.x+dx,cur.y};
    if(pointInPolygon(onlyX,outline.points)) return onlyX;
    glm::vec2 onlyZ={cur.x,cur.y+dz};
    if(pointInPolygon(onlyZ,outline.points)) return onlyZ;
    return cur;
}

int findCursorOnWall(glm::vec2 pos,float threshold=0.12f)
{
    int n=(int)outline.points.size();
    int best=-1; float bestDist=threshold;
    for(int i=0;i<n;i++){
        glm::vec2 A=outline.points[i];
        glm::vec2 B=outline.points[(i+1)%n];
        glm::vec2 AB=B-A; float len=glm::length(AB);
        if(len<0.001f) continue;
        glm::vec2 dir=AB/len;
        float t=glm::clamp(glm::dot(pos-A,dir),0.0f,len);
        if(t<0.0001f||t>len-0.0001f) continue;
        glm::vec2 closest=A+dir*t;
        float dist=glm::length(pos-closest);
        if(dist<bestDist){bestDist=dist;best=i;}
    }
    return best;
}

glm::vec2 moveAlongWall(glm::vec2 pos,int wallIdx,float dist)
{
    int n=(int)outline.points.size();
    glm::vec2 A=outline.points[wallIdx];
    glm::vec2 B=outline.points[(wallIdx+1)%n];
    glm::vec2 AB=B-A; float len=glm::length(AB);
    if(len<0.005f) return pos;
    glm::vec2 dir=AB/len;
    float t=glm::dot(pos-A,dir);
    t=glm::clamp(t,0.002f,len-0.002f);
    float newT=glm::clamp(t+dist,0.002f,len-0.002f);
    return A+dir*newT;
}

glm::vec2 moveAlongWallWithHop(glm::vec2 pos,int wallIdx,float dist)
{
    glm::vec2 np=moveAlongWall(pos,wallIdx,dist);
    float moved=glm::length(np-pos);
    float remaining=std::abs(dist)-moved;
    if(remaining>0.001f){
        int n=(int)outline.points.size();
        glm::vec2 wallDir=glm::normalize(outline.points[(wallIdx+1)%n]-outline.points[wallIdx]);
        if(dist<0) wallDir=-wallDir;
        glm::vec2 hop=moveIwCursor(np,wallDir.x*remaining,wallDir.y*remaining);
        if(glm::length(hop-np)>0.0001f) return hop;
    }
    return np;
}

glm::vec2 screenToWorld(double sx,double sy)
{
    float ndcX=(float)(2.0*sx/SCR_WIDTH)-1.0f;
    float ndcY=1.0f-(float)(2.0*sy/SCR_HEIGHT);
    float halfH=camera.topDownZoom*0.5f,halfW=halfH*(float)SCR_WIDTH/SCR_HEIGHT;
    return {camera.position.x+ndcX*halfW,camera.position.z-ndcY*halfH};
}

glm::vec2 snapToGrid(glm::vec2 p,float g)
{return {std::round(p.x/g)*g,std::round(p.y/g)*g};}

unsigned int makeSimpleVAO(unsigned int& vbo,size_t n,GLenum usage)
{
    unsigned int vao;
    glGenVertexArrays(1,&vao);glGenBuffers(1,&vbo);
    glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);
    std::vector<float> z(n,0.f);
    glBufferData(GL_ARRAY_BUFFER,n*sizeof(float),z.data(),usage);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);glBindVertexArray(0);
    return vao;
}

static void deleteVAOVBO(unsigned int& vao,unsigned int& vbo)
{
    if(vao){glDeleteVertexArrays(1,&vao);vao=0;}
    if(vbo){glDeleteBuffers(1,&vbo);vbo=0;}
}

unsigned int outlineLinesVAO=0,outlineLinesVBO=0;
unsigned int outlinePointsVAO=0,outlinePointsVBO=0;
unsigned int outlineFillVAO=0,outlineFillVBO=0;
unsigned int previewLineVAO=0,previewLineVBO=0;
unsigned int anglePreviewVAO=0,anglePreviewVBO=0;
unsigned int snapDotVAO=0,snapDotVBO=0;
unsigned int angleDotVAO=0,angleDotVBO=0;
unsigned int iwCursorVAO=0,iwCursorVBO=0;
unsigned int iwLinesVAO=0,iwLinesVBO=0;
unsigned int snapHighlightVAO=0,snapHighlightVBO=0;
int outlineLinesCount=0,outlinePointsCount=0,outlineFillCount=0,iwLinesCount=0;
bool outlineGLDirty=false;

unsigned int floorVAO=0,floorVBO=0,floorEBO=0,gridVAO=0;
int gridCount=0;
unsigned int previewQuadVAO=0,previewQuadVBO=0;
int previewQuadCount=0;

void buildPreviewQuad(glm::vec2 wA,glm::vec2 wB,
                      float offset,float width,
                      float yBottom,float yTop,
                      glm::vec2 surfaceOffset={0,0},
                      float insetFwd=0.02f)
{
    if(previewQuadVAO){glDeleteVertexArrays(1,&previewQuadVAO);previewQuadVAO=0;}
    if(previewQuadVBO){glDeleteBuffers(1,&previewQuadVBO);previewQuadVBO=0;}
    glm::vec2 d=wB-wA; float len=glm::length(d);
    if(len<0.001f) return;
    glm::vec2 dir=d/len, perp={-dir.y,dir.x};
    glm::vec2 sA=wA+surfaceOffset;
    glm::vec2 pA=sA+dir*offset+perp*insetFwd;
    glm::vec2 pB=sA+dir*(offset+width)+perp*insetFwd;
    float verts[]={
        pA.x,yBottom,pA.y, pB.x,yBottom,pB.y,
        pB.x,yBottom,pB.y, pB.x,yTop,   pB.y,
        pB.x,yTop,   pB.y, pA.x,yTop,   pA.y,
        pA.x,yTop,   pA.y, pA.x,yBottom,pA.y,
    };
    previewQuadCount=8;
    glGenVertexArrays(1,&previewQuadVAO);glGenBuffers(1,&previewQuadVBO);
    glBindVertexArray(previewQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER,previewQuadVBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(verts),verts,GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);glBindVertexArray(0);
}

void buildFloor(float size)
{
    if(floorVAO)glDeleteVertexArrays(1,&floorVAO);
    if(floorVBO)glDeleteBuffers(1,&floorVBO);
    if(floorEBO)glDeleteBuffers(1,&floorEBO);
    float h=size*0.5f;
    float v[]={-h,0,-h,0,1,0,0,0,h,0,-h,0,1,0,1,0,h,0,h,0,1,0,1,1,-h,0,h,0,1,0,0,1};
    unsigned int idx[]={0,1,2,0,2,3};
    glGenVertexArrays(1,&floorVAO);glGenBuffers(1,&floorVBO);glGenBuffers(1,&floorEBO);
    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER,floorVBO);glBufferData(GL_ARRAY_BUFFER,sizeof(v),v,GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,floorEBO);glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(idx),idx,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)0);glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(6*sizeof(float)));glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void buildGrid(float size)
{
    if(gridVAO)glDeleteVertexArrays(1,&gridVAO);
    std::vector<float> v; float half=size*0.5f;
    for(float z=-half;z<=half+0.001f;z+=1)v.insert(v.end(),{-half,0,z,half,0,z});
    for(float x=-half;x<=half+0.001f;x+=1)v.insert(v.end(),{x,0,-half,x,0,half});
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

void rebuildIwLinesGL(glm::vec2 cursor)
{
    deleteVAOVBO(iwLinesVAO,iwLinesVBO);
    std::vector<float> lv;
    for(size_t i=0;i+1<iwPoints.size();++i){
        lv.insert(lv.end(),{iwPoints[i].x,0.03f,iwPoints[i].y});
        lv.insert(lv.end(),{iwPoints[i+1].x,0.03f,iwPoints[i+1].y});
    }
    glm::vec2 from=iwPoints.empty()?cursor:iwPoints.back();
    lv.insert(lv.end(),{from.x,0.03f,from.y});
    lv.insert(lv.end(),{cursor.x,0.03f,cursor.y});
    iwLinesCount=(int)(lv.size()/3);
    if(!lv.empty()){
        iwLinesVAO=makeSimpleVAO(iwLinesVBO,lv.size(),GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,iwLinesVBO);
        glBufferData(GL_ARRAY_BUFFER,lv.size()*sizeof(float),lv.data(),GL_DYNAMIC_DRAW);
    }
}

void updateIwCursorGL(glm::vec2 pos,float s=0.15f)
{
    float h=s*0.5f;
    float d[]={
        pos.x-h,0.04f,pos.y-h, pos.x+h,0.04f,pos.y-h,
        pos.x+h,0.04f,pos.y-h, pos.x+h,0.04f,pos.y+h,
        pos.x+h,0.04f,pos.y+h, pos.x-h,0.04f,pos.y+h,
        pos.x-h,0.04f,pos.y+h, pos.x-h,0.04f,pos.y-h,
    };
    glBindBuffer(GL_ARRAY_BUFFER,iwCursorVBO);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(d),d);
}

void updateSnapHighlightGL(const std::vector<glm::vec2>& pts)
{
    deleteVAOVBO(snapHighlightVAO,snapHighlightVBO);
    if(pts.empty()) return;
    std::vector<float> v;
    for(auto& p:pts) v.insert(v.end(),{p.x,0.04f,p.y});
    snapHighlightVAO=makeSimpleVAO(snapHighlightVBO,v.size(),GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER,snapHighlightVBO);
    glBufferData(GL_ARRAY_BUFFER,v.size()*sizeof(float),v.data(),GL_DYNAMIC_DRAW);
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

void updateAnglePreview(glm::vec2 next,bool valid)
{
    if(!valid||outline.points.empty()||outline.closed) return;
    float d[]={outline.points.back().x,0.015f,outline.points.back().y,next.x,0.015f,next.y};
    glBindBuffer(GL_ARRAY_BUFFER,anglePreviewVBO);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(d),d);
    float dd[]={next.x,0.025f,next.y};
    glBindBuffer(GL_ARRAY_BUFFER,angleDotVBO);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(dd),dd);
}

void buildWalls(float h)
{
    walls.clear();
    glm::vec2 c=outline.centroid();
    for(size_t i=0;i<outline.points.size();++i){
        size_t j=(i+1)%outline.points.size();
        walls.emplace_back(outline.points[i],outline.points[j],h);
        walls.back().buildWithCutouts(c,getCutoutsForOutlineWall((int)i));
    }
}

void tryAddOutlinePoint(glm::vec2 pos)
{
    if(panel.state!=AppState::DRAWING_OUTLINE) return;
    if(outline.isNearStart(pos)&&outline.canClose()){
        outline.tryClose();panel.state=AppState::CONFIRM_OUTLINE;outlineGLDirty=true;
    } else {outline.addPoint(pos);outlineGLDirty=true;}
}

void finalizeInnerWalls(float thickness,float height)
{
    if(iwPoints.size()<2) return;
    float h=(height<0.01f)?apartmentHeight:height;
    for(size_t i=0;i+1<iwPoints.size();++i){
        glm::vec2 A=iwPoints[i],B=iwPoints[i+1];
        glm::vec2 d=B-A; float len=glm::length(d);
        if(len<0.05f) continue;
        float ang=glm::degrees(std::atan2(d.y,d.x));
        innerWalls.emplace_back(A,len,ang,thickness,h);
    }
}

void getWallGeom(bool isInner,int idx,
                 glm::vec2& outA,glm::vec2& outB,glm::vec2& outInward)
{
    if(!isInner){
        const auto& pts=outline.points;
        int n=(int)pts.size();
        outA=pts[idx]; outB=pts[(idx+1)%n];
        glm::vec2 d=outB-outA;
        float len=glm::length(d);
        glm::vec2 dir=(len>0.001f)?d/len:glm::vec2(1,0);
        glm::vec2 n1={-dir.y,dir.x},n2={dir.y,-dir.x};
        glm::vec2 mid=(outA+outB)*0.5f;
        glm::vec2 c=outline.centroid();
        outInward=(glm::dot(n1,c-mid)>0)?n1:n2;
    } else {
        const auto& iw=innerWalls[idx];
        float rad=glm::radians(iw.angle);
        glm::vec2 dir={std::cos(rad),std::sin(rad)};
        outA=iw.startXZ;
        outB=iw.startXZ+dir*iw.length;
        outInward={-std::sin(rad),std::cos(rad)};
    }
}

glm::vec2 getInnerWallSurfaceOffset(int iwIdx)
{
    if(iwIdx<0||iwIdx>=(int)innerWalls.size()) return {0,0};
    const InnerWall& iw=innerWalls[iwIdx];
    float rad=glm::radians(iw.angle);
    glm::vec2 perp={-std::sin(rad),std::cos(rad)};
    return perp*(iw.thickness*0.5f);
}

void framebuffer_size_callback(GLFWwindow*,int w,int h){glViewport(0,0,w,h);}
void scroll_callback(GLFWwindow*,double,double y){camera.processMouseScroll((float)y);}

void mouse_callback(GLFWwindow*,double xpos,double ypos)
{
    cursorScreenX=xpos;cursorScreenY=ypos;
    if(!mouseCapured) return;
    if(firstMouse){lastMouseX=(float)xpos;lastMouseY=(float)ypos;firstMouse=false;}
    camera.processMouseMove((float)xpos-lastMouseX,lastMouseY-(float)ypos);
    lastMouseX=(float)xpos;lastMouseY=(float)ypos;
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
    bool wt=ImGui::GetIO().WantTextInput;

    if(key==GLFW_KEY_ESCAPE){
        if(furniturePlacementMode){furniturePlacementMode=false;return;}
        if(builtinPlacementMode){builtinPlacementMode=false;return;}
        if(furnitureMoveMode){furnitureMoveMode=false;return;}
        if(iwState!=IWState::IDLE){
            iwState=IWState::IDLE;iwPoints.clear();
            deleteVAOVBO(snapHighlightVAO,snapHighlightVBO);
            return;
        }
        if(mouseCapured){
            glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_NORMAL);
            mouseCapured=false;firstMouse=true;
        } else glfwSetWindowShouldClose(window,true);
    }
    if(key==GLFW_KEY_1){
        camera.setMode(CameraMode::TOP_DOWN);
        glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_NORMAL);mouseCapured=false;
    }
    if(key==GLFW_KEY_2){
        if(panel.state!=AppState::DESIGN_MODE) return;
        camera.setMode(CameraMode::FREE);
        glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
        mouseCapured=true;firstMouse=true;
    }
    if(key==GLFW_KEY_3){
        if(panel.state!=AppState::DESIGN_MODE) return;
        glm::vec2 c=outline.centroid();
        camera.setMode(CameraMode::EXPLORE);
        camera.position=glm::vec3(c.x,1.7f,c.y);
        glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
        mouseCapured=true;firstMouse=true;
    }
    if(!wt){
        if(key==GLFW_KEY_SPACE) pendingSpace=true;
        if(key==GLFW_KEY_ENTER||key==GLFW_KEY_KP_ENTER) pendingEnter=true;
        if(key==GLFW_KEY_Z){
            if(panel.state==AppState::DRAWING_OUTLINE){
                outline.removeLast();outlineGLDirty=true;
            } else if(iwState==IWState::PLACING&&!iwPoints.empty()){
                iwPoints.pop_back();
            }
        }
        if(key==GLFW_KEY_R&&furniturePlacementMode){
            furniturePlacingRotY+=45.0f;
            if(furniturePlacingRotY>=360.0f) furniturePlacingRotY-=360.0f;
        }
        if(key==GLFW_KEY_R&&builtinPlacementMode){
            builtinPlacingRotY+=45.0f;
            if(builtinPlacingRotY>=360.0f) builtinPlacingRotY-=360.0f;
        }
    }
}

int main(int argc,char* argv[])
{
    if(!SDL_Init(SDL_INIT_AUDIO|SDL_INIT_EVENTS)){std::cerr<<"SDL failed\n";return -1;}
    if(!glfwInit()){std::cerr<<"GLFW failed\n";SDL_Quit();return -1;}
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window=glfwCreateWindow(SCR_WIDTH,SCR_HEIGHT,"Interior Designer 3D",nullptr,nullptr);
    if(!window){glfwTerminate();SDL_Quit();return -1;}
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);
    glfwSetCursorPosCallback(window,mouse_callback);
    glfwSetScrollCallback(window,scroll_callback);
    glfwSetKeyCallback(window,key_callback);
    glfwSetMouseButtonCallback(window,mouse_button_callback);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){glfwTerminate();SDL_Quit();return -1;}

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
    Shader flatShader ("shaders/flat.vert","shaders/flat.frag");

    phongShader.use();
    phongShader.setInt("diffuseTexture",0);
    phongShader.setInt("useTexture",0);

    buildFloor(30.0f);buildGrid(30.0f);
    scanModels();
    std::cout<<"[Models] Znaleziono: "<<availableModels.size()<<" plikow OBJ\n";
    std::cout<<"[Furniture] Biblioteka wbudowana: "<<(int)FurnitureType::COUNT<<" typow\n";

    auto initDyn=[](unsigned int& vao,unsigned int& vbo,size_t n){
        glGenVertexArrays(1,&vao);glGenBuffers(1,&vbo);
        glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,vbo);
        std::vector<float> z(n,0.f);
        glBufferData(GL_ARRAY_BUFFER,n*sizeof(float),z.data(),GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
        glEnableVertexAttribArray(0);glBindVertexArray(0);
    };
    initDyn(previewLineVAO,previewLineVBO,6);
    initDyn(snapDotVAO,snapDotVBO,3);
    initDyn(anglePreviewVAO,anglePreviewVBO,6);
    initDyn(angleDotVAO,angleDotVBO,3);
    initDyn(iwCursorVAO,iwCursorVBO,24);

    glm::vec3 lightDir=glm::normalize(glm::vec3(0.5f,1.0f,0.3f));
    glm::vec3 lAmb(0.35f),lDif(0.8f,0.8f,0.75f),lSpc(0.3f);

    auto setPhong=[&](const glm::mat4& model,glm::vec3 color){
        phongShader.setMat4("model",model);
        phongShader.setVec3("objectColor",color);
        phongShader.setVec3("lightDirection",lightDir);
        phongShader.setVec3("lightAmbient",lAmb);
        phongShader.setVec3("lightDiffuse",lDif);
        phongShader.setVec3("lightSpecular",lSpc);
        phongShader.setFloat("shininess",32.0f);
        phongShader.setInt("useTexture",0);
    };

    while(!glfwWindowShouldClose(window))
    {
        float now=(float)glfwGetTime();
        deltaTime=now-lastFrame;lastFrame=now;

        if(camera.getMode()==CameraMode::EXPLORE&&panel.state==AppState::DESIGN_MODE
           &&outline.points.size()>=3){
            glm::vec3 old=camera.position;
            camera.processKeyboard(window,deltaTime);
            glm::vec2 res=resolveCollisions({camera.position.x,camera.position.z});
            if(!pointInPolygon(res,outline.points)){camera.position.x=old.x;camera.position.z=old.z;}
            else{camera.position.x=res.x;camera.position.z=res.y;}
            camera.position.y=1.7f;
        } else camera.processKeyboard(window,deltaTime);

        for(auto& d:doors) d.update(deltaTime,camera.position);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glm::vec2 rawC=screenToWorld(cursorScreenX,cursorScreenY);
        glm::vec2 snapC=snapToGrid(rawC,panel.snapGrid);
        if(!outline.points.empty()&&outline.isNearStart(snapC)) snapC=outline.points[0];

        std::vector<float> innerLens;
        for(auto& iw:innerWalls) innerLens.push_back(iw.length);

        int   iwOnWallIdx=-1;
        float iwOnWallLen=0.0f;
        if(iwState==IWState::PLACING&&outline.points.size()>=3){
            iwOnWallIdx=findCursorOnWall(iwCursor,0.12f);
            if(iwOnWallIdx>=0) iwOnWallLen=outline.edgeLength(iwOnWallIdx);
        }

        IWMoveFlags moveFlags;
        if(iwState==IWState::PLACING&&outline.points.size()>=3){
            float step=panel.iw_moveAmount;
            auto tryMove=[&](float dx,float dz)->bool{
                glm::vec2 np=moveIwCursor(iwCursor,dx,dz);
                return glm::length(np-iwCursor)>0.001f;
            };
            moveFlags.canLeft  = tryMove(-step,0);
            moveFlags.canRight = tryMove(+step,0);
            moveFlags.canUp    = tryMove(0,-step);
            moveFlags.canDown  = tryMove(0,+step);
            moveFlags.onWall   = (iwOnWallIdx>=0);
            moveFlags.wallIdx  = iwOnWallIdx;
            if(iwOnWallIdx>=0){
                glm::vec2 npL=moveAlongWallWithHop(iwCursor,iwOnWallIdx,-step);
                glm::vec2 npR=moveAlongWallWithHop(iwCursor,iwOnWallIdx,+step);
                moveFlags.canAlongLeft  = glm::length(npL-iwCursor)>0.001f;
                moveFlags.canAlongRight = glm::length(npR-iwCursor)>0.001f;
            }
        }

        // Buduj etykiety mebli
        std::vector<std::string> furnitureLabels;
        for(auto& item:furnitureManager.items) furnitureLabels.push_back(item.modelFile);

        glm::vec3 selPos={0,0,0};
        float selRotY=0.0f;
        if(selectedFurnitureIdx>=0&&selectedFurnitureIdx<(int)furnitureManager.items.size()){
            selPos=furnitureManager.items[selectedFurnitureIdx].position;
            selRotY=furnitureManager.items[selectedFurnitureIdx].rotationY;
        }

        PanelResult pr=panel.render(
            outline,camera.getMode(),snapC,
            (int)innerWalls.size(),(int)doors.size(),(int)roomWindows.size(),
            iwState,iwCursor,(int)iwPoints.size(),
            innerLens,iwOnWallIdx,iwOnWallLen,moveFlags,
            availableModels,
            furnitureLabels,
            selectedFurnitureIdx,
            selPos,
            selRotY,
            furniturePlacementMode,
            builtinPlacementMode,
            furnitureMoveMode);

        // ── IW logika ─────────────────────────────────────────────────

        if(pr.iwBeginSelectOrigin&&iwState==IWState::IDLE){
            iwState=IWState::SELECT_ORIGIN;
            iwPoints.clear();
            updateSnapHighlightGL(getSnapPoints());
        }
        if(pr.iwCancel){
            iwState=IWState::IDLE;iwPoints.clear();
            deleteVAOVBO(snapHighlightVAO,snapHighlightVBO);
        }

        if(!ImGui::GetIO().WantCaptureMouse&&pendingLeftClick
           &&iwState==IWState::SELECT_ORIGIN
           &&camera.getMode()==CameraMode::TOP_DOWN){
            auto pts=getSnapPoints();
            float dist; glm::vec2 nearest=findNearest(snapC,pts,dist);
            if(dist<1.5f){
                iwCursor=nearest;
                iwState=IWState::PLACING;iwPoints.clear();
                deleteVAOVBO(snapHighlightVAO,snapHighlightVBO);
                updateIwCursorGL(iwCursor);
                pendingLeftClick=false;
            }
        }

        if(iwState==IWState::PLACING){
            bool moved=false;
            if(pr.iw_moveX!=0||pr.iw_moveZ!=0){
                glm::vec2 np=moveIwCursor(iwCursor,pr.iw_moveX,pr.iw_moveZ);
                if(glm::length(np-iwCursor)>0.0001f){iwCursor=np;moved=true;}
            }
            if(pr.iw_moveAlong!=0&&iwOnWallIdx>=0){
                glm::vec2 np=moveAlongWallWithHop(iwCursor,iwOnWallIdx,pr.iw_moveAlong);
                if(glm::length(np-iwCursor)>0.0001f){iwCursor=np;moved=true;}
            }
            if(moved){updateIwCursorGL(iwCursor);rebuildIwLinesGL(iwCursor);}
        }

        bool doPlace=pr.iwPlacePoint||
            (pendingEnter&&!ImGui::GetIO().WantTextInput&&iwState==IWState::PLACING);
        if(doPlace&&iwState==IWState::PLACING){
            iwPoints.push_back(iwCursor);
            rebuildIwLinesGL(iwCursor);
            pendingEnter=false;
        }

        if(pr.iwUndo&&iwState==IWState::PLACING&&!iwPoints.empty()){
            iwPoints.pop_back();rebuildIwLinesGL(iwCursor);
        }

        bool doCreate=pr.iwCreateWalls||
            (pendingSpace&&!ImGui::GetIO().WantTextInput
             &&iwState==IWState::PLACING&&(int)iwPoints.size()>=2);
        if(doCreate&&iwState==IWState::PLACING&&(int)iwPoints.size()>=2){
            finalizeInnerWalls(pr.iw_thickness,pr.iw_height);
            iwState=IWState::IDLE;iwPoints.clear();
            deleteVAOVBO(iwLinesVAO,iwLinesVBO);
            pendingSpace=false;
        }

        // ── Meble OBJ ─────────────────────────────────────────────────

        if(pr.startFurniturePlacement&&!availableModels.empty()){
            furniturePlacementMode    = true;
            furniturePlacingModelIdx  = pr.furnitureModelIdx;
            furniturePlacingRotY      = pr.furnitureRotY;
            furniturePlacingScale     = pr.furnitureScale;
        }
        if(pr.cancelFurniturePlacement){
            furniturePlacementMode=false;
            builtinPlacementMode=false;
            furnitureMoveMode=false;
        }

        // ── Meble wbudowane ───────────────────────────────────────────

        if(pr.startBuiltinPlacement){
            builtinPlacementMode  = true;
            builtinPlacingType    = (FurnitureType)pr.builtinFurnitureType;
            builtinPlacingRotY    = pr.builtinRotY;
            builtinPlacingScale   = pr.builtinScale;
        }

        // ── Faza 5 — selekcja i edycja ────────────────────────────────

        // Selekcja z panelu
        if(pr.clickedFurnitureIdx>=-1){
            selectedFurnitureIdx=pr.clickedFurnitureIdx;
            furnitureMoveMode=false;
        }
        // Przesuwanie z panelu
        if(pr.requestMoveFurniture&&selectedFurnitureIdx>=0)
            furnitureMoveMode=true;
        // Edycja obrotu zaznaczonego
        if(pr.newFurnitureRotY>-998.f
           &&selectedFurnitureIdx>=0
           &&selectedFurnitureIdx<(int)furnitureManager.items.size())
            furnitureManager.items[selectedFurnitureIdx].rotationY=pr.newFurnitureRotY;

        // ── Klikniecia mebli OBJ / wbudowanych ───────────────────────

        if(!ImGui::GetIO().WantCaptureMouse&&pendingLeftClick
           &&furniturePlacementMode
           &&camera.getMode()==CameraMode::TOP_DOWN
           &&panel.state==AppState::DESIGN_MODE){
            if(furniturePlacingModelIdx<(int)availableModels.size()){
                std::string modelFile   = availableModels[furniturePlacingModelIdx];
                std::string textureFile = findTexture(modelFile);
                glm::vec3 pos = {snapC.x, 0.0f, snapC.y};
                furnitureManager.addItem(modelFile,textureFile,
                    pos,furniturePlacingRotY,furniturePlacingScale);
            }
            furniturePlacementMode=false;
            pendingLeftClick=false;
        }

        if(!ImGui::GetIO().WantCaptureMouse&&pendingLeftClick
           &&builtinPlacementMode
           &&camera.getMode()==CameraMode::TOP_DOWN
           &&panel.state==AppState::DESIGN_MODE){
            glm::vec3 pos={snapC.x, furnitureMountHeight(builtinPlacingType), snapC.y};
            furnitureManager.addBuiltinItem(builtinPlacingType,pos,builtinPlacingRotY,builtinPlacingScale);
            builtinPlacementMode=false;
            pendingLeftClick=false;
        }

        // Przesuniecie zaznaczonego mebla
        if(furnitureMoveMode
           &&selectedFurnitureIdx>=0
           &&selectedFurnitureIdx<(int)furnitureManager.items.size()
           &&pendingLeftClick&&!ImGui::GetIO().WantCaptureMouse
           &&camera.getMode()==CameraMode::TOP_DOWN){
            furnitureManager.items[selectedFurnitureIdx].position.x=snapC.x;
            furnitureManager.items[selectedFurnitureIdx].position.z=snapC.y;
            furnitureMoveMode=false;
            pendingLeftClick=false;
        }

        // Selekcja kliknieciem w viewport
        if(!furniturePlacementMode&&!builtinPlacementMode&&!furnitureMoveMode
           &&pendingLeftClick&&!ImGui::GetIO().WantCaptureMouse
           &&camera.getMode()==CameraMode::TOP_DOWN
           &&panel.state==AppState::DESIGN_MODE){
            float bestDist=0.9f; int bestIdx=-1;
            for(int i=0;i<(int)furnitureManager.items.size();++i){
                glm::vec2 fp(furnitureManager.items[i].position.x,
                             furnitureManager.items[i].position.z);
                float d=glm::length(snapC-fp);
                if(d<bestDist){bestDist=d;bestIdx=i;}
            }
            if(bestIdx>=0){selectedFurnitureIdx=bestIdx;pendingLeftClick=false;}
            else selectedFurnitureIdx=-1;
        }

        // Usuwanie mebla
        if(pr.deleteFurniture>=0&&pr.deleteFurniture<(int)furnitureManager.items.size()){
            furnitureManager.items.erase(furnitureManager.items.begin()+pr.deleteFurniture);
            if(selectedFurnitureIdx==pr.deleteFurniture) selectedFurnitureIdx=-1;
            else if(selectedFurnitureIdx>pr.deleteFurniture) selectedFurnitureIdx--;
            furnitureMoveMode=false;
        }

        // ── Klikniecia obrysu ─────────────────────────────────────────

        if(!ImGui::GetIO().WantCaptureMouse){
            if(pendingLeftClick&&panel.state==AppState::DRAWING_OUTLINE
               &&camera.getMode()==CameraMode::TOP_DOWN)
                tryAddOutlinePoint(snapC);
            if(pendingRightClick&&panel.state==AppState::DRAWING_OUTLINE){
                outline.removeLast();outlineGLDirty=true;
            }
        }
        pendingLeftClick=false;pendingRightClick=false;

        if((pendingSpace||pr.placeNextPoint)
           &&panel.state==AppState::DRAWING_OUTLINE&&pr.hasNextPoint){
            tryAddOutlinePoint(pr.nextPoint);panel.inputAngle=0;pendingSpace=false;
        }
        pendingSpace=false;pendingEnter=false;

        if(pr.undoPoint){outline.removeLast();outlineGLDirty=true;}
        if(pr.clearOutline){
            outline.clear();walls.clear();innerWalls.clear();
            doors.clear();roomWindows.clear();furnitureManager.clear();
            iwState=IWState::IDLE;iwPoints.clear();
            furniturePlacementMode=false;builtinPlacementMode=false;furnitureMoveMode=false;
            selectedFurnitureIdx=-1;
            deleteVAOVBO(snapHighlightVAO,snapHighlightVBO);
            deleteVAOVBO(iwLinesVAO,iwLinesVBO);
            panel.state=AppState::DRAWING_OUTLINE;outlineGLDirty=true;
        }
        if(pr.closeOutline&&outline.canClose()){
            outline.tryClose();panel.state=AppState::CONFIRM_OUTLINE;outlineGLDirty=true;
        }
        if(pr.confirmOutline){
            apartmentHeight=pr.apartmentHeight;
            buildWalls(apartmentHeight);
            panel.state=AppState::DESIGN_MODE;
        }
        if(pr.reopenOutline){
            outline.reopen();walls.clear();innerWalls.clear();
            doors.clear();roomWindows.clear();furnitureManager.clear();
            iwState=IWState::IDLE;iwPoints.clear();
            furniturePlacementMode=false;builtinPlacementMode=false;furnitureMoveMode=false;
            selectedFurnitureIdx=-1;
            deleteVAOVBO(snapHighlightVAO,snapHighlightVBO);
            deleteVAOVBO(iwLinesVAO,iwLinesVBO);
            panel.state=AppState::DRAWING_OUTLINE;outlineGLDirty=true;
            camera.setMode(CameraMode::TOP_DOWN);
            glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_NORMAL);mouseCapured=false;
        }
        if(pr.floorSizeDirty){buildFloor(pr.floorSize);buildGrid(pr.floorSize);}

        if(pr.deleteInnerWall>=0&&pr.deleteInnerWall<(int)innerWalls.size()){
            innerWalls.erase(innerWalls.begin()+pr.deleteInnerWall);
            wallsDirty=true;
        }

        if(pr.placeDoor||pr.placePassage){
            glm::vec2 wA,wB,wIn;
            getWallGeom(pr.door_isInner,pr.door_wallIdx,wA,wB,wIn);
            if(pr.door_isInner&&pr.door_wallIdx<(int)innerWalls.size()){
                glm::vec2 surf=getInnerWallSurfaceOffset(pr.door_wallIdx);
                wA+=surf;wB+=surf;
            }
            Door::Type dt=pr.door_swing?Door::Type::SWING:Door::Type::SLIDING;
            Door newDoor(wA,wB,wIn,
                         pr.door_offset,pr.door_width,pr.door_height,
                         dt,pr.door_openLeft,pr.placePassage,apartmentHeight);
            newDoor.wallIndex   = pr.door_wallIdx;
            newDoor.isInnerWall = pr.door_isInner;
            doors.push_back(std::move(newDoor));
            wallsDirty=true;
        }
        if(pr.deleteDoor>=0&&pr.deleteDoor<(int)doors.size()){
            doors.erase(doors.begin()+pr.deleteDoor);wallsDirty=true;
        }

        if(pr.placeWindow){
            glm::vec2 wA,wB,wIn;
            getWallGeom(pr.win_isInner,pr.win_wallIdx,wA,wB,wIn);
            glm::vec2 surfOffset={0,0};
            if(pr.win_isInner&&pr.win_wallIdx<(int)innerWalls.size())
                surfOffset=getInnerWallSurfaceOffset(pr.win_wallIdx);
            RoomWindow rw;
            rw.wallIndex=pr.win_wallIdx; rw.isInnerWall=pr.win_isInner;
            rw.offset=pr.win_offset; rw.width=pr.win_width;
            rw.height=pr.win_height; rw.sill=pr.win_sill;
            glm::vec2 dir=glm::normalize(wB-wA);
            glm::vec2 wXA=wA+dir*pr.win_offset+surfOffset;
            glm::vec2 wXB=wA+dir*(pr.win_offset+pr.win_width)+surfOffset;
            glm::vec2 gXA=wA+dir*pr.win_offset;
            glm::vec2 gXB=wA+dir*(pr.win_offset+pr.win_width);
            float y0=pr.win_sill,y1=pr.win_sill+pr.win_height;
            float frameV[]={
                wXA.x,y0,wXA.y, wXA.x,y1,wXA.y,
                wXB.x,y0,wXB.y, wXB.x,y1,wXB.y,
                wXA.x,y0,wXA.y, wXB.x,y0,wXB.y,
                wXA.x,y1,wXA.y, wXB.x,y1,wXB.y,
            };
            rw.frameVertCount=8;
            glGenVertexArrays(1,&rw.frameVAO);glGenBuffers(1,&rw.frameVBO);
            glBindVertexArray(rw.frameVAO);
            glBindBuffer(GL_ARRAY_BUFFER,rw.frameVBO);
            glBufferData(GL_ARRAY_BUFFER,sizeof(frameV),frameV,GL_STATIC_DRAW);
            glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
            glEnableVertexAttribArray(0);glBindVertexArray(0);
            float glassV[]={
                gXA.x,y0+0.01f,gXA.y, gXB.x,y0+0.01f,gXB.y, gXB.x,y1-0.01f,gXB.y,
                gXA.x,y0+0.01f,gXA.y, gXB.x,y1-0.01f,gXB.y, gXA.x,y1-0.01f,gXA.y,
            };
            rw.glassVertCount=6;
            glGenVertexArrays(1,&rw.glassVAO);glGenBuffers(1,&rw.glassVBO);
            glBindVertexArray(rw.glassVAO);
            glBindBuffer(GL_ARRAY_BUFFER,rw.glassVBO);
            glBufferData(GL_ARRAY_BUFFER,sizeof(glassV),glassV,GL_STATIC_DRAW);
            glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
            glEnableVertexAttribArray(0);glBindVertexArray(0);
            roomWindows.push_back(std::move(rw));
            wallsDirty=true;
        }
        if(pr.deleteWindow>=0&&pr.deleteWindow<(int)roomWindows.size()){
            roomWindows.erase(roomWindows.begin()+pr.deleteWindow);wallsDirty=true;
        }

        if(wallsDirty&&!walls.empty()) rebuildAllWalls();

        bool showDoorPreview=(panel.state==AppState::DESIGN_MODE
            &&panel.activeTool==Panel::DesignTool::DOOR
            &&camera.getMode()==CameraMode::TOP_DOWN
            &&iwState==IWState::IDLE);
        bool showWindowPreview=(panel.state==AppState::DESIGN_MODE
            &&panel.activeTool==Panel::DesignTool::WINDOW
            &&camera.getMode()==CameraMode::TOP_DOWN
            &&iwState==IWState::IDLE);

        if(showDoorPreview||showWindowPreview){
            bool  pIsInner = showDoorPreview?panel.door_isInner:panel.win_isInner;
            int   pWallIdx = showDoorPreview?panel.door_wallIdx:panel.win_wallIdx;
            float pOffset  = showDoorPreview?panel.door_offset:panel.win_offset;
            float pWidth   = showDoorPreview?panel.door_width:panel.win_width;
            float pYBottom = showDoorPreview?0.0f:panel.win_sill;
            float pYTop    = showDoorPreview?panel.door_height:panel.win_sill+panel.win_height;
            glm::vec2 wA={0,0},wB={1,0},surfOff={0,0};
            bool validWall=false;
            if(!pIsInner&&pWallIdx<(int)outline.points.size()){
                int n=(int)outline.points.size();
                wA=outline.points[pWallIdx];wB=outline.points[(pWallIdx+1)%n];
                validWall=true;
            } else if(pIsInner&&pWallIdx<(int)innerWalls.size()){
                float rad=glm::radians(innerWalls[pWallIdx].angle);
                glm::vec2 dir={std::cos(rad),std::sin(rad)};
                wA=innerWalls[pWallIdx].startXZ;
                wB=wA+dir*innerWalls[pWallIdx].length;
                surfOff=getInnerWallSurfaceOffset(pWallIdx);
                validWall=true;
            }
            if(validWall){
                float wallLen=glm::length(wB-wA);
                if(pOffset+pWidth<=wallLen)
                    buildPreviewQuad(wA,wB,pOffset,pWidth,pYBottom,pYTop,surfOff,0.02f);
                else if(previewQuadVAO){glDeleteVertexArrays(1,&previewQuadVAO);previewQuadVAO=0;}
            }
        } else if(previewQuadVAO){
            glDeleteVertexArrays(1,&previewQuadVAO);previewQuadVAO=0;
        }

        if(outlineGLDirty){rebuildOutlineGL();outlineGLDirty=false;}
        if(panel.state==AppState::DRAWING_OUTLINE){
            updateMousePreview(snapC);
            updateAnglePreview(pr.nextPoint,pr.hasNextPoint);
        }
        if(iwState==IWState::PLACING){
            updateIwCursorGL(iwCursor);
            rebuildIwLinesGL(iwCursor);
        }

        // ── Renderowanie ──────────────────────────────────────────────

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

        // Podloga
        phongShader.use();
        phongShader.setMat4("view",view);phongShader.setMat4("projection",proj);
        phongShader.setVec3("viewPos",camera.position);
        phongShader.setVec3("lightDirection",lightDir);
        phongShader.setVec3("lightAmbient",lAmb);
        phongShader.setVec3("lightDiffuse",lDif);
        phongShader.setVec3("lightSpecular",lSpc);
        phongShader.setFloat("shininess",32.0f);
        setPhong(I,{0.55f,0.50f,0.45f});
        glBindVertexArray(floorVAO);glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);

        // Siatka
        flatShader.use();setFlatMVP();
        float gc=(camera.getMode()==CameraMode::TOP_DOWN)?0.45f:0.25f;
        flatShader.setVec4("color",{gc,gc,gc,1});
        glBindVertexArray(gridVAO);glDrawArrays(GL_LINES,0,gridCount);

        // Sciany zewnetrzne
        if(!walls.empty()){
            phongShader.use();
            phongShader.setMat4("view",view);phongShader.setMat4("projection",proj);
            phongShader.setVec3("viewPos",camera.position);
            setPhong(I,{0.88f,0.85f,0.80f});
            for(auto& w:walls) w.draw();
        }

        // Sciany wewnetrzne
        if(!innerWalls.empty()){
            phongShader.use();
            phongShader.setMat4("view",view);phongShader.setMat4("projection",proj);
            phongShader.setVec3("viewPos",camera.position);
            setPhong(I,{0.82f,0.78f,0.72f});
            for(auto& iw:innerWalls) iw.draw();
        }

        // Drzwi
        if(!doors.empty()){
            flatShader.use();setFlatMVP();
            flatShader.setVec4("color",{0.4f,0.25f,0.1f,1.0f});
            glLineWidth(2.0f);
            for(auto& d:doors) d.drawFrame();
            glLineWidth(1.0f);
            phongShader.use();
            phongShader.setMat4("view",view);phongShader.setMat4("projection",proj);
            phongShader.setVec3("viewPos",camera.position);
            setPhong(I,{0.65f,0.45f,0.2f});
            for(auto& d:doors) d.drawPanel();
        }

        // Okna
        if(!roomWindows.empty()){
            flatShader.use();setFlatMVP();
            glLineWidth(2.0f);
            flatShader.setVec4("color",{0.8f,0.8f,0.8f,1.0f});
            for(auto& rw:roomWindows) rw.drawFrame();
            glLineWidth(1.0f);
            flatShader.setVec4("color",{0.5f,0.8f,1.0f,0.3f});
            glDepthMask(GL_FALSE);
            for(auto& rw:roomWindows) rw.drawGlass();
            glDepthMask(GL_TRUE);
        }

        // Preview drzwi/okna
        if(previewQuadVAO&&previewQuadCount>0){
            flatShader.use();setFlatMVP();
            glLineWidth(3.0f);
            flatShader.setVec4("color",showDoorPreview
                ?glm::vec4(1.0f,0.6f,0.1f,0.9f)
                :glm::vec4(0.2f,0.9f,1.0f,0.9f));
            glBindVertexArray(previewQuadVAO);
            glDrawArrays(GL_LINES,0,previewQuadCount);
            glLineWidth(1.0f);
        }

        // Meble
        if(!furnitureManager.items.empty()){
            phongShader.use();
            phongShader.setMat4("view",view);
            phongShader.setMat4("projection",proj);
            phongShader.setVec3("viewPos",camera.position);
            phongShader.setVec3("lightDirection",lightDir);
            phongShader.setVec3("lightAmbient",lAmb);
            phongShader.setVec3("lightDiffuse",lDif);
            phongShader.setVec3("lightSpecular",lSpc);
            phongShader.setFloat("shininess",32.0f);
            furnitureManager.draw(phongShader);
        }

        // Wskaznik zaznaczonego mebla (zolty krzyzyk)
        if(selectedFurnitureIdx>=0&&selectedFurnitureIdx<(int)furnitureManager.items.size()){
            flatShader.use();setFlatMVP();
            float sx=furnitureManager.items[selectedFurnitureIdx].position.x;
            float sz=furnitureManager.items[selectedFurnitureIdx].position.z;
            float s=0.55f;
            float sv[]={sx-s,0.08f,sz, sx+s,0.08f,sz, sx,0.08f,sz-s, sx,0.08f,sz+s};
            unsigned int tv,tb;
            glGenVertexArrays(1,&tv);glGenBuffers(1,&tb);
            glBindVertexArray(tv);glBindBuffer(GL_ARRAY_BUFFER,tb);
            glBufferData(GL_ARRAY_BUFFER,sizeof(sv),sv,GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
            glEnableVertexAttribArray(0);
            glLineWidth(3.0f);
            flatShader.setVec4("color",{1.0f,1.0f,0.0f,1.0f});
            glDrawArrays(GL_LINES,0,4);
            glLineWidth(1.0f);
            glBindVertexArray(0);
            glDeleteVertexArrays(1,&tv);glDeleteBuffers(1,&tb);

            // Wskaznik trybu przesuwania (zielony krzyzyk na kursorze)
            if(furnitureMoveMode&&camera.getMode()==CameraMode::TOP_DOWN){
                float cx=snapC.x,cz=snapC.y;
                float mv[]={cx-s,0.09f,cz, cx+s,0.09f,cz, cx,0.09f,cz-s, cx,0.09f,cz+s};
                glGenVertexArrays(1,&tv);glGenBuffers(1,&tb);
                glBindVertexArray(tv);glBindBuffer(GL_ARRAY_BUFFER,tb);
                glBufferData(GL_ARRAY_BUFFER,sizeof(mv),mv,GL_DYNAMIC_DRAW);
                glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
                glEnableVertexAttribArray(0);
                glLineWidth(2.0f);
                flatShader.setVec4("color",{0.2f,1.0f,0.5f,0.9f});
                glDrawArrays(GL_LINES,0,4);
                glLineWidth(1.0f);
                glBindVertexArray(0);
                glDeleteVertexArrays(1,&tv);glDeleteBuffers(1,&tb);
            }
        }

        // Obrys
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
            flatShader.setVec4("color",{0.7f,0.7f,0.7f,0.4f});
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
            bool ns=outline.isNearStart(snapC)&&outline.canClose();
            flatShader.setVec4("color",ns?glm::vec4(0.1f,1.0f,0.3f,1.0f):glm::vec4(0.0f,0.9f,1.0f,0.7f));
            glBindVertexArray(snapDotVAO);glDrawArrays(GL_POINTS,0,1);glPointSize(1.0f);
        }

        if(iwState==IWState::SELECT_ORIGIN&&snapHighlightVAO){
            glPointSize(16.0f);flatShader.setVec4("color",{1.0f,1.0f,0.0f,1.0f});
            glBindVertexArray(snapHighlightVAO);
            glDrawArrays(GL_POINTS,0,(int)getSnapPoints().size());glPointSize(1.0f);
        }
        if(iwState==IWState::PLACING){
            if(iwLinesVAO&&iwLinesCount>0){
                glLineWidth(2.0f);flatShader.setVec4("color",{0.0f,0.9f,1.0f,0.8f});
                glBindVertexArray(iwLinesVAO);glDrawArrays(GL_LINES,0,iwLinesCount);glLineWidth(1.0f);
            }
            if(!iwPoints.empty()){
                std::vector<float> pv;
                for(auto& p:iwPoints) pv.insert(pv.end(),{p.x,0.04f,p.y});
                unsigned int tv,tb;
                glGenVertexArrays(1,&tv);glGenBuffers(1,&tb);
                glBindVertexArray(tv);glBindBuffer(GL_ARRAY_BUFFER,tb);
                glBufferData(GL_ARRAY_BUFFER,pv.size()*sizeof(float),pv.data(),GL_DYNAMIC_DRAW);
                glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
                glEnableVertexAttribArray(0);
                glPointSize(10.0f);flatShader.setVec4("color",{1,1,1,1});
                glDrawArrays(GL_POINTS,0,(int)iwPoints.size());glPointSize(1.0f);
                glBindVertexArray(0);
                glDeleteVertexArrays(1,&tv);glDeleteBuffers(1,&tb);
            }
            glLineWidth(2.5f);
            flatShader.setVec4("color",moveFlags.onWall
                ?glm::vec4(1.0f,0.85f,0.1f,1.0f)
                :glm::vec4(0.1f,0.4f,1.0f,1.0f));
            glBindVertexArray(iwCursorVAO);glDrawArrays(GL_LINES,0,8);glLineWidth(1.0f);
        }

        // Overlay: OBJ placement
        if(furniturePlacementMode&&camera.getMode()==CameraMode::TOP_DOWN){
            ImGui::SetNextWindowPos({SCR_WIDTH/2-150.0f,10},ImGuiCond_Always);
            ImGui::SetNextWindowSize({300,60},ImGuiCond_Always);
            ImGuiWindowFlags hf=ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|
                ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoTitleBar|
                ImGuiWindowFlags_NoFocusOnAppearing;
            ImGui::Begin("##fhint",nullptr,hf);
            if(!availableModels.empty()&&furniturePlacingModelIdx<(int)availableModels.size())
                ImGui::TextColored({0.2f,1.0f,0.4f,1.0f},
                    "Kliknij LPM: %s",availableModels[furniturePlacingModelIdx].c_str());
            ImGui::TextDisabled("R = obrot 45 deg | ESC = anuluj");
            ImGui::End();
        }

        // Overlay: builtin placement
        if(builtinPlacementMode&&camera.getMode()==CameraMode::TOP_DOWN){
            float yPos=furniturePlacementMode?80.0f:10.0f;
            ImGui::SetNextWindowPos({SCR_WIDTH/2-150.0f,yPos},ImGuiCond_Always);
            ImGui::SetNextWindowSize({300,60},ImGuiCond_Always);
            ImGuiWindowFlags hf=ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|
                ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoTitleBar|
                ImGuiWindowFlags_NoFocusOnAppearing;
            ImGui::Begin("##bhint",nullptr,hf);
            ImGui::TextColored({0.2f,1.0f,0.8f,1.0f},
                "Kliknij LPM: %s",furnitureName(builtinPlacingType));
            ImGui::TextDisabled("R = obrot 45 deg | ESC = anuluj");
            ImGui::End();
        }

        // Overlay: move mode
        if(furnitureMoveMode&&camera.getMode()==CameraMode::TOP_DOWN){
            ImGui::SetNextWindowPos({SCR_WIDTH/2-150.0f,10},ImGuiCond_Always);
            ImGui::SetNextWindowSize({300,50},ImGuiCond_Always);
            ImGuiWindowFlags hf=ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|
                ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoTitleBar|
                ImGuiWindowFlags_NoFocusOnAppearing;
            ImGui::Begin("##mvhint",nullptr,hf);
            ImGui::TextColored({0.2f,1.0f,0.5f,1.0f},"Kliknij nowe miejsce dla mebla");
            ImGui::TextDisabled("ESC = anuluj");
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    walls.clear();innerWalls.clear();doors.clear();roomWindows.clear();
    furnitureManager.clear();
    if(floorVAO)glDeleteVertexArrays(1,&floorVAO);
    if(floorVBO)glDeleteBuffers(1,&floorVBO);
    if(floorEBO)glDeleteBuffers(1,&floorEBO);
    if(gridVAO)glDeleteVertexArrays(1,&gridVAO);
    if(previewQuadVAO)glDeleteVertexArrays(1,&previewQuadVAO);
    if(previewQuadVBO)glDeleteBuffers(1,&previewQuadVBO);
    deleteVAOVBO(outlineLinesVAO,outlineLinesVBO);
    deleteVAOVBO(outlinePointsVAO,outlinePointsVBO);
    deleteVAOVBO(outlineFillVAO,outlineFillVBO);
    deleteVAOVBO(previewLineVAO,previewLineVBO);
    deleteVAOVBO(snapDotVAO,snapDotVBO);
    deleteVAOVBO(anglePreviewVAO,anglePreviewVBO);
    deleteVAOVBO(angleDotVAO,angleDotVBO);
    deleteVAOVBO(iwCursorVAO,iwCursorVBO);
    deleteVAOVBO(iwLinesVAO,iwLinesVBO);
    deleteVAOVBO(snapHighlightVAO,snapHighlightVBO);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);glfwTerminate();SDL_Quit();
    return 0;
}