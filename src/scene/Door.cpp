#include "Door.h"
#include "scene/Outline.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

void Door::initFromWall(glm::vec2 A, glm::vec2 B, glm::vec2 inward,
                        float off, float w, float h,
                        Type t, bool oLeft, bool passage, float aptH)
{
    offset   = off;
    width    = w;
    height   = h;
    type     = t;
    openLeft = oLeft;
    isPassage = passage;
    wInward  = inward;

    glm::vec2 d   = B - A;
    float     len = glm::length(d);
    wStart = A;
    wDir   = (len > 0.001f) ? d/len : glm::vec2(1,0);

    hingeXZ = openLeft ? (A + wDir*off) : (A + wDir*(off+w));

    glm::vec2 cXZ = A + wDir*(off + w*0.5f);
    worldCenter = {cXZ.x, h*0.5f, cXZ.y};

    // Przejscie — od razu otwarte
    if (isPassage) {
        openAngle   = 90.0f;
        targetAngle = 90.0f;
    }

    buildFrame();
    initPanel();
}

Door::Door(int wallIdx, float off, float w, float h,
           Type t, bool oLeft, bool passage,
           const Outline& outline, float aptHeight)
{
    wallIndex   = wallIdx;
    isInnerWall = false;

    const auto& pts = outline.points;
    int n = (int)pts.size();
    glm::vec2 A = pts[wallIdx];
    glm::vec2 B = pts[(wallIdx+1)%n];
    glm::vec2 d = B - A;
    float len   = glm::length(d);
    glm::vec2 dir = (len>0.001f) ? d/len : glm::vec2(1,0);

    glm::vec2 n1 = {-dir.y, dir.x};
    glm::vec2 n2 = { dir.y,-dir.x};
    glm::vec2 mid = (A+B)*0.5f;
    glm::vec2 c   = outline.centroid();
    glm::vec2 inward = (glm::dot(n1,c-mid)>0) ? n1 : n2;

    initFromWall(A,B,inward,off,w,h,t,oLeft,passage,aptHeight);
}

Door::Door(glm::vec2 wallA, glm::vec2 wallB, glm::vec2 inward,
           float off, float w, float h,
           Type t, bool oLeft, bool passage, float aptHeight)
{
    wallIndex   = -1;
    isInnerWall = true;
    initFromWall(wallA,wallB,inward,off,w,h,t,oLeft,passage,aptHeight);
}

Door::Door(Door&& o) noexcept
    : wStart(o.wStart),wDir(o.wDir),wInward(o.wInward),hingeXZ(o.hingeXZ)
    , worldCenter(o.worldCenter)
    , wallIndex(o.wallIndex),isInnerWall(o.isInnerWall)
    , offset(o.offset),width(o.width),height(o.height)
    , type(o.type),openLeft(o.openLeft),isPassage(o.isPassage)
    , openAngle(o.openAngle),targetAngle(o.targetAngle)
    , frameVAO(o.frameVAO),frameVBO(o.frameVBO)
    , panelVAO(o.panelVAO),panelVBO(o.panelVBO)
    , frameVertCount(o.frameVertCount)
{
    o.frameVAO=0;o.frameVBO=0;o.panelVAO=0;o.panelVBO=0;
}

Door& Door::operator=(Door&& o) noexcept {
    if(this!=&o){
        if(frameVAO) glDeleteVertexArrays(1,&frameVAO);
        if(frameVBO) glDeleteBuffers(1,&frameVBO);
        if(panelVAO) glDeleteVertexArrays(1,&panelVAO);
        if(panelVBO) glDeleteBuffers(1,&panelVBO);
        wStart=o.wStart;wDir=o.wDir;wInward=o.wInward;hingeXZ=o.hingeXZ;
        worldCenter=o.worldCenter;
        wallIndex=o.wallIndex;isInnerWall=o.isInnerWall;
        offset=o.offset;width=o.width;height=o.height;
        type=o.type;openLeft=o.openLeft;isPassage=o.isPassage;
        openAngle=o.openAngle;targetAngle=o.targetAngle;
        frameVAO=o.frameVAO;frameVBO=o.frameVBO;
        panelVAO=o.panelVAO;panelVBO=o.panelVBO;
        frameVertCount=o.frameVertCount;
        o.frameVAO=0;o.frameVBO=0;o.panelVAO=0;o.panelVBO=0;
    }
    return *this;
}

Door::~Door(){
    if(frameVAO) glDeleteVertexArrays(1,&frameVAO);
    if(frameVBO) glDeleteBuffers(1,&frameVBO);
    if(panelVAO) glDeleteVertexArrays(1,&panelVAO);
    if(panelVBO) glDeleteBuffers(1,&panelVBO);
}

void Door::buildFrame(){
    glm::vec2 leftXZ  = wStart + wDir*offset;
    glm::vec2 rightXZ = wStart + wDir*(offset+width);

    float verts[] = {
        leftXZ.x,0,leftXZ.y,   leftXZ.x,height,leftXZ.y,
        rightXZ.x,0,rightXZ.y, rightXZ.x,height,rightXZ.y,
        leftXZ.x,height,leftXZ.y, rightXZ.x,height,rightXZ.y,
    };
    frameVertCount=6;

    glGenVertexArrays(1,&frameVAO);glGenBuffers(1,&frameVBO);
    glBindVertexArray(frameVAO);
    glBindBuffer(GL_ARRAY_BUFFER,frameVBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(verts),verts,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void Door::initPanel(){
    glGenVertexArrays(1,&panelVAO);glGenBuffers(1,&panelVBO);
    glBindVertexArray(panelVAO);
    glBindBuffer(GL_ARRAY_BUFFER,panelVBO);
    glBufferData(GL_ARRAY_BUFFER,18*sizeof(float),nullptr,GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    updatePanel();
}

void Door::updatePanel(){
    if(isPassage) return;  // przejscie nie ma skrzydla

    float verts[18];
    if(type==Type::SWING){
        float theta   = glm::radians(openAngle);
        glm::vec2 dir = openLeft ? wDir : -wDir;
        glm::vec2 freeXZ = hingeXZ +
            (std::cos(theta)*dir + std::sin(theta)*wInward)*width;

        verts[0]=hingeXZ.x;verts[1]=0;       verts[2]=hingeXZ.y;
        verts[3]=freeXZ.x; verts[4]=0;       verts[5]=freeXZ.y;
        verts[6]=freeXZ.x; verts[7]=height;  verts[8]=freeXZ.y;
        verts[9]=hingeXZ.x;verts[10]=0;      verts[11]=hingeXZ.y;
        verts[12]=freeXZ.x;verts[13]=height; verts[14]=freeXZ.y;
        verts[15]=hingeXZ.x;verts[16]=height;verts[17]=hingeXZ.y;
    } else {
        float progress = openAngle/90.0f;
        float startOff = offset + progress*width;
        glm::vec2 pA = wStart + wDir*startOff;
        glm::vec2 pB = wStart + wDir*(startOff+width);

        verts[0]=pA.x;verts[1]=0;      verts[2]=pA.y;
        verts[3]=pB.x;verts[4]=0;      verts[5]=pB.y;
        verts[6]=pB.x;verts[7]=height; verts[8]=pB.y;
        verts[9]=pA.x;verts[10]=0;     verts[11]=pA.y;
        verts[12]=pB.x;verts[13]=height;verts[14]=pB.y;
        verts[15]=pA.x;verts[16]=height;verts[17]=pA.y;
    }
    glBindBuffer(GL_ARRAY_BUFFER,panelVBO);
    glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(verts),verts);
}

void Door::update(float dt, glm::vec3 playerPos){
    if(isPassage){
        // Przejscie zawsze otwarte — nie animuje sie
        openAngle   = 90.0f;
        targetAngle = 90.0f;
        return;
    }

    glm::vec3 diff = playerPos - worldCenter;
    targetAngle = (glm::dot(diff,diff) < TRIGGER_DIST*TRIGGER_DIST) ? 90.0f : 0.0f;

    float delta = targetAngle - openAngle;
    if(std::abs(delta)>0.01f){
        float step = OPEN_SPEED*dt;
        openAngle += (delta>0?1.0f:-1.0f)*std::min(step,std::abs(delta));
        updatePanel();
    }
}

void Door::drawFrame() const{
    glBindVertexArray(frameVAO);
    glDrawArrays(GL_LINES,0,frameVertCount);
    glBindVertexArray(0);
}

void Door::drawPanel() const{
    if(isPassage) return;
    glBindVertexArray(panelVAO);
    glDrawArrays(GL_TRIANGLES,0,6);
    glBindVertexArray(0);
}