#include "InnerWall.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <algorithm>
#include <cmath>

InnerWall::InnerWall(glm::vec2 start, float len, float angleDeg,
                     float thick, float h)
    : startXZ(start), length(len), angle(angleDeg)
    , thickness(thick), height(h)
{ build(); }

InnerWall::InnerWall(InnerWall&& o) noexcept
    : startXZ(o.startXZ), length(o.length), angle(o.angle)
    , thickness(o.thickness), height(o.height)
    , cornersXZ(o.cornersXZ)
    , VAO(o.VAO), VBO(o.VBO), vertexCount(o.vertexCount)
{ o.VAO=0; o.VBO=0; }

InnerWall& InnerWall::operator=(InnerWall&& o) noexcept {
    if(this!=&o){
        if(VAO) glDeleteVertexArrays(1,&VAO);
        if(VBO) glDeleteBuffers(1,&VBO);
        startXZ=o.startXZ; length=o.length; angle=o.angle;
        thickness=o.thickness; height=o.height;
        cornersXZ=o.cornersXZ;
        VAO=o.VAO; VBO=o.VBO; vertexCount=o.vertexCount;
        o.VAO=0; o.VBO=0;
    }
    return *this;
}

InnerWall::~InnerWall() {
    if(VAO) glDeleteVertexArrays(1,&VAO);
    if(VBO) glDeleteBuffers(1,&VBO);
}

void InnerWall::uploadVerts(const std::vector<float>& verts) {
    if(VAO){ glDeleteVertexArrays(1,&VAO); glDeleteBuffers(1,&VBO); }
    vertexCount=(int)(verts.size()/8);
    if(!vertexCount) return;
    glGenVertexArrays(1,&VAO); glGenBuffers(1,&VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,verts.size()*sizeof(float),verts.data(),GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void InnerWall::addQuad(std::vector<float>& verts,
                         glm::vec2 A, glm::vec2 B,
                         float y0, float y1, glm::vec3 n)
{
    auto add=[&](glm::vec2 p,float y,float u,float v){
        verts.insert(verts.end(),{p.x,y,p.y,n.x,n.y,n.z,u,v});
    };
    add(A,y0,0,0); add(B,y0,1,0); add(B,y1,1,1);
    add(A,y0,0,0); add(B,y1,1,1); add(A,y1,0,1);
}

void InnerWall::addFaceWithCutouts(std::vector<float>& verts,
                                    glm::vec2 A, glm::vec2 B,
                                    float wallLen,
                                    glm::vec3 normal,
                                    const std::vector<WallCutout>& cutouts)
{
    if(wallLen<0.001f) return;
    glm::vec2 dir=(B-A)/wallLen;

    std::vector<WallCutout> sorted=cutouts;
    std::sort(sorted.begin(),sorted.end(),
        [](const WallCutout& a,const WallCutout& b){ return a.offsetStart<b.offsetStart; });

    float cur=0.0f;
    for(const auto& c:sorted){
        float cs=glm::clamp(c.offsetStart,0.0f,wallLen);
        float ce=glm::clamp(c.offsetEnd,  0.0f,wallLen);
        float cb=glm::clamp(c.yBottom,    0.0f,height);
        float ct=glm::clamp(c.yTop,       0.0f,height);
        if(ce<=cs) continue;

        if(cur<cs)
            addQuad(verts,A+dir*cur,A+dir*cs,0,height,normal);

        if(cb>0.001f)
            addQuad(verts,A+dir*cs,A+dir*ce,0,cb,normal);
        if(ct<height-0.001f)
            addQuad(verts,A+dir*cs,A+dir*ce,ct,height,normal);

        cur=ce;
    }
    if(cur<wallLen-0.001f)
        addQuad(verts,A+dir*cur,A+dir*wallLen,0,height,normal);
}

void InnerWall::build() { buildWithCutouts({}); }

void InnerWall::buildWithCutouts(const std::vector<WallCutout>& cutouts)
{
    float rad=glm::radians(angle);
    glm::vec2 dir  ={std::cos(rad),std::sin(rad)};
    glm::vec2 perp ={-std::sin(rad),std::cos(rad)};
    float ht=thickness*0.5f;

    glm::vec2 SL=startXZ+perp*ht;
    glm::vec2 SR=startXZ-perp*ht;
    glm::vec2 EL=startXZ+dir*length+perp*ht;
    glm::vec2 ER=startXZ+dir*length-perp*ht;
    cornersXZ={SL,SR,EL,ER};

    glm::vec3 nL={perp.x,0,perp.y};
    glm::vec3 nR={-perp.x,0,-perp.y};
    glm::vec3 nS={-dir.x,0,-dir.y};
    glm::vec3 nE={dir.x,0,dir.y};
    glm::vec3 nUp={0,1,0};

    std::vector<float> verts;
    verts.reserve(8*8*6);

    // ── Lewa sciana SL→EL (kierunek = +dir) ─────────────────────────
    addFaceWithCutouts(verts, SL, EL, length, nL, cutouts);

    // ── Prawa sciana ER→SR (kierunek = -dir) ─────────────────────────
    // KLUCZOWE: cutouty sa mierzone od POCZATKU sciany (SR/SL strona).
    // Prawa sciana idzie od ER do SR czyli odwrotnie.
    // Offset 0 w kierunku prawej sciany = ER = KONIEC wallu.
    // Dlatego [cs,ce] → [length-ce, length-cs]
    std::vector<WallCutout> mirroredCuts;
    mirroredCuts.reserve(cutouts.size());
    for(const auto& c:cutouts)
        mirroredCuts.push_back({length-c.offsetEnd, length-c.offsetStart,
                                 c.yBottom, c.yTop});
    addFaceWithCutouts(verts, ER, SR, length, nR, mirroredCuts);

    // ── Czolo i tyl (krotkie, bez wyciec) ────────────────────────────
    addQuad(verts,SR,SL,0,height,nS);
    addQuad(verts,EL,ER,0,height,nE);

    // ── Gora ─────────────────────────────────────────────────────────
    {
        auto add=[&](glm::vec2 p,float y,float u,float v){
            verts.insert(verts.end(),{p.x,y,p.y,nUp.x,nUp.y,nUp.z,u,v});
        };
        add(SL,height,0,0); add(SR,height,1,0); add(ER,height,1,1);
        add(SL,height,0,0); add(ER,height,1,1); add(EL,height,0,1);
    }

    uploadVerts(verts);
}

void InnerWall::draw() const {
    if(!VAO) return;
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES,0,vertexCount);
    glBindVertexArray(0);
}

std::array<std::pair<glm::vec2,glm::vec2>,4> InnerWall::getEdges() const {
    return {
        std::make_pair(cornersXZ[0],cornersXZ[2]),
        std::make_pair(cornersXZ[2],cornersXZ[3]),
        std::make_pair(cornersXZ[3],cornersXZ[1]),
        std::make_pair(cornersXZ[1],cornersXZ[0]),
    };
}