// Co-op / presentation: Render co-op HUD, effects, display modes, and capture diagnostics.
#pragma once
#ifndef COOP_PARTNER_MARKER_H
#define COOP_PARTNER_MARKER_H
#include "coop_hud.h"
#include <math.h>

struct CoOpMarkerCamera {
    double position[3];
    short rotation[3][3]; // retail SCamera.Transform, 12-bit fixed point
    double focalX,focalY,centerX,centerY;
};
struct CoOpPartnerMarker {
    bool valid,onScreen;
    float x,y,directionX,directionY,scale,bodyPixels,uiUnit,displayScale;
    // Initialize co-op partner marker with no active retail ownership.
    CoOpPartnerMarker():valid(false),onScreen(false),x(0),y(0),
        // Prepare direction x for the owning viewport.
        directionX(0),directionY(0),scale(0),bodyPixels(0),
        // Prepare ui unit for the owning viewport.
        uiUnit(0),displayScale(1) {}
};
// Prepare marker clamp for the owning viewport.
inline float CoOp_MarkerClamp(float value,float low,float high) {
    if(value<low)return low;if(value>high)return high;return value;
}
// Prepare partner head target for the owning viewport.
inline void CoOp_PartnerHeadTarget(const double origin[3],
    const short rotation[3][3],double head[3]) {
    // The model's local -Y runs from its origin toward its head. Rotate that
    // vector with the player's own transform so wall/ceiling poses follow it.
    double up[3]={-rotation[0][1]/4096.0,-rotation[1][1]/4096.0,
        -rotation[2][1]/4096.0};
    double length=sqrt(up[0]*up[0]+up[1]*up[1]+up[2]*up[2]);
    if(length<.5 || length>1.5) {
        up[0]=up[2]=0;up[1]=-1;length=1;
    }
    for(unsigned axis=0;axis<3;++axis)head[axis]=origin[axis]+64.0*up[axis]/length;
}
// Prepare marker gap for the owning viewport.
inline float CoOp_MarkerGap(float unit) {
    return unit*3.0f>3.0f?unit*3.0f:3.0f;
}
// Prepare marker label unit for the owning viewport.
inline float CoOp_MarkerLabelUnit(float projectedScale,float displayScale) {
    // Size from the full display, not one pane: a 1080p glyph cell is at
    // least 4.5 screen pixels in either split orientation.
    return CoOp_MarkerClamp(projectedScale*1.5f,2.0f,2.6f)*displayScale;
}
// Prepare marker arrow height for the owning viewport.
inline float CoOp_MarkerArrowHeight(float unit,float displayScale) {
    return CoOp_MarkerClamp(5.5f*unit,9.0f*displayScale,13.0f*displayScale);
}
// Prepare marker glyph row for the owning viewport.
inline unsigned char CoOp_MarkerGlyphRow(char glyph,unsigned row) {
    static const unsigned char p[7]={30,17,17,30,16,16,16};
    static const unsigned char one[7]={4,12,4,4,4,4,14};
    static const unsigned char two[7]={14,17,1,2,4,8,31};
    if(row>=7)return 0;
    return glyph=='P'?p[row]:glyph=='1'?one[row]:two[row];
}
// Return project partner from the owning viewport without changing the shared world.
inline CoOpPartnerMarker CoOp_ProjectPartner(const CoOpMarkerCamera& camera,
    const double target[3],const CoOpHudPane& pane,float displayHeight=480.0f) {
    CoOpPartnerMarker result;
    if(pane.w<40 || pane.h<40 || camera.focalX<=0 || camera.focalY<=0)return result;
    result.displayScale=CoOp_MarkerClamp(displayHeight/480.0f,1.0f,2.5f);
    double relative[3]={target[0]-camera.position[0],target[1]-camera.position[1],
        target[2]-camera.position[2]};
    double view[3]={0,0,0};
    // SCamera.Transform columns are camera axes in world space (the retail
    // narrow-view centering code uses column 0 as camera-right). Project by
    // taking the dot product with each column, not each row.
    for(int axis=0;axis<3;++axis)for(int world=0;world<3;++world)
        view[axis]+=relative[world]*camera.rotation[world][axis]/4096.0;
    if(view[0]>1e8 || view[0]<-1e8 || view[1]>1e8 || view[1]<-1e8 ||
        view[2]>1e8 || view[2]<-1e8)return result;
    double centerX=pane.x+pane.w*.5,centerY=pane.y+pane.h*.5;
    double screenX=centerX,screenY=centerY;
    if(view[2]>1.0) {
        screenX=camera.centerX+camera.focalX*view[0]/view[2];
        screenY=camera.centerY+camera.focalY*view[1]/view[2];
        result.bodyPixels=(float)(camera.focalY*96.0/view[2]);
        result.scale=CoOp_MarkerClamp(result.bodyPixels/65.0f,.15f,1.6f);
        result.uiUnit=CoOp_MarkerLabelUnit(result.scale,result.displayScale);
        float margin=9.0f*result.displayScale;
        float unit=result.uiUnit;
        float above=CoOp_MarkerGap(unit)+
            CoOp_MarkerArrowHeight(unit,result.displayScale)+10*unit+3;
        if(screenX>=pane.x+margin && screenX<=pane.x+pane.w-margin &&
           screenY>=pane.y+above && screenY<=pane.y+pane.h-margin) {
            result.valid=result.onScreen=true;
            result.x=(float)screenX;result.y=(float)screenY;
            return result;
        }
    }
    double dx=view[2]>1.0?screenX-centerX:-view[0];
    double dy=view[2]>1.0?screenY-centerY:-view[1];
    if(dx<.001 && dx>-.001 && dy<.001 && dy>-.001)dy=1;
    double length=sqrt(dx*dx+dy*dy);
    if(length<=0)return result;
    dx/=length;dy/=length;
    float inset=14.0f*result.displayScale;
    double absX=dx<0?-dx:dx,absY=dy<0?-dy:dy;
    double horizontal=absX>.000001?(pane.w*.5-inset)/absX:1e30;
    double vertical=absY>.000001?(pane.h*.5-inset)/absY:1e30;
    double travel=horizontal<vertical?horizontal:vertical;
    result.valid=true;result.onScreen=false;
    result.x=(float)(centerX+dx*travel);result.y=(float)(centerY+dy*travel);
    result.directionX=(float)dx;result.directionY=(float)dy;
    return result;
}
template<unsigned Capacity> inline void CoOp_MarkerQuad(CoOpHudBuffer<Capacity>& buffer,
    unsigned owner,float x0,float y0,float x1,float y1,float x2,float y2,
    float x3,float y3,unsigned long color) {
    CoOpHudCommand command;memset(&command,0,sizeof(command));
    command.owner=owner;command.style=HUD_PROJECTED;
    command.texture=1;command.blend=1;
    // Negative 2D depths are sent behind the world by PCGfx_DrawQuad2D.
    // The retail HUD uses a small positive near-layer depth instead.
    command.quad.depth=.05f;
    // Retail's DX7 quad is a triangle fan: 0,1,2,3 around the perimeter.
    // Callers give rectangle corners as TL,TR,BL,BR; swap the last two.
    float xs[4]={x0,x1,x3,x2},ys[4]={y0,y1,y3,y2};
    for(unsigned i=0;i<4;++i) {
        command.quad.v[i].x=xs[i];command.quad.v[i].y=ys[i];
        command.quad.v[i].u=(i==1 || i==2)?1.0f:0.0f;
        command.quad.v[i].v=i>=2?1.0f:0.0f;
        command.quad.v[i].color=color;
    }
    buffer.Add(command);
}
template<unsigned Capacity> inline void CoOp_MarkerRect(CoOpHudBuffer<Capacity>& buffer,
    unsigned owner,float x,float y,float w,float h,unsigned long color) {
    CoOp_MarkerQuad(buffer,owner,x,y,x+w,y,x,y+h,x+w,y+h,color);
}
template<unsigned Capacity> inline void CoOp_QueuePartnerMarker(CoOpHudBuffer<Capacity>& buffer,
    const CoOpPartnerMarker& marker,const CoOpHudPane& pane,unsigned viewer) {
    if(!marker.valid || viewer<1 || viewer>2)return;
    unsigned partner=viewer==1?2:1;
    unsigned long color=partner==1?0xFFE95152:0xFF51D8F2;
    if(!marker.onScreen) {
        float dx=marker.directionX,dy=marker.directionY;
        float sideX=-dy,sideY=dx,size=12.0f*marker.displayScale;
        float baseX=marker.x-dx*size;
        float baseY=marker.y-dy*size;
        CoOp_MarkerQuad(buffer,viewer,baseX+sideX*size*.7f,baseY+sideY*size*.7f,
            baseX-sideX*size*.7f,baseY-sideY*size*.7f,
            marker.x,marker.y,marker.x,marker.y,color);
        return;
    }
    float unit=marker.uiUnit;
    float tipY=marker.y-CoOp_MarkerGap(unit);
    float baseY=tipY-CoOp_MarkerArrowHeight(unit,marker.displayScale);
    float labelX=CoOp_MarkerClamp(marker.x-6.5f*unit,pane.x+2*unit,
        pane.x+pane.w-15*unit);
    float labelY=baseY-10*unit;
    // 5x7 glyphs are deliberately drawn as whole visible pixels: P1 or P2.
    // DXPOLY prepends same-layer polygons, then renders in reverse queue
    // order. Submit letters before their stripe and background so the letters
    // are the final visible layer rather than being covered by the badge.
    for(unsigned row=0;row<7;++row)for(unsigned col=0;col<5;++col) {
        if(CoOp_MarkerGlyphRow('P',row)&(16>>col))
            CoOp_MarkerRect(buffer,viewer,labelX+(1+col)*unit,
                labelY+(1+row)*unit,unit,unit,0xFFF8FBFF);
        if(CoOp_MarkerGlyphRow(partner==1?'1':'2',row)&(16>>col))
            CoOp_MarkerRect(buffer,viewer,labelX+(7+col)*unit,
                labelY+(1+row)*unit,unit,unit,0xFFF8FBFF);
    }
    CoOp_MarkerRect(buffer,viewer,labelX,labelY,13*unit,unit,color);
    CoOp_MarkerRect(buffer,viewer,labelX,labelY,13*unit,9*unit,0xFF111820);
    CoOp_MarkerQuad(buffer,viewer,marker.x-3*unit,baseY,
        marker.x+3*unit,baseY,marker.x,tipY,marker.x,tipY,color);
}
#endif
