#include "RoomWindow.h"
#include <glad/glad.h>

RoomWindow::RoomWindow(RoomWindow&& o) noexcept
    : wallIndex(o.wallIndex), isInnerWall(o.isInnerWall)
    , offset(o.offset), width(o.width), height(o.height), sill(o.sill)
    , frameVAO(o.frameVAO), frameVBO(o.frameVBO)
    , glassVAO(o.glassVAO), glassVBO(o.glassVBO)
    , frameVertCount(o.frameVertCount), glassVertCount(o.glassVertCount)
{
    o.frameVAO=0; o.frameVBO=0; o.glassVAO=0; o.glassVBO=0;
}

RoomWindow& RoomWindow::operator=(RoomWindow&& o) noexcept
{
    if(this!=&o){
        if(frameVAO) glDeleteVertexArrays(1,&frameVAO);
        if(frameVBO) glDeleteBuffers(1,&frameVBO);
        if(glassVAO) glDeleteVertexArrays(1,&glassVAO);
        if(glassVBO) glDeleteBuffers(1,&glassVBO);
        wallIndex=o.wallIndex; isInnerWall=o.isInnerWall;
        offset=o.offset; width=o.width; height=o.height; sill=o.sill;
        frameVAO=o.frameVAO; frameVBO=o.frameVBO;
        glassVAO=o.glassVAO; glassVBO=o.glassVBO;
        frameVertCount=o.frameVertCount; glassVertCount=o.glassVertCount;
        o.frameVAO=0; o.frameVBO=0; o.glassVAO=0; o.glassVBO=0;
    }
    return *this;
}

RoomWindow::~RoomWindow()
{
    if(frameVAO) glDeleteVertexArrays(1,&frameVAO);
    if(frameVBO) glDeleteBuffers(1,&frameVBO);
    if(glassVAO) glDeleteVertexArrays(1,&glassVAO);
    if(glassVBO) glDeleteBuffers(1,&glassVBO);
}

void RoomWindow::drawFrame() const
{
    if(!frameVAO) return;
    glBindVertexArray(frameVAO);
    glDrawArrays(GL_LINES,0,frameVertCount);
    glBindVertexArray(0);
}

void RoomWindow::drawGlass() const
{
    if(!glassVAO) return;
    glBindVertexArray(glassVAO);
    glDrawArrays(GL_TRIANGLES,0,glassVertCount);
    glBindVertexArray(0);
}