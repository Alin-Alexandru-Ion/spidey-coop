// Co-op / presentation: Render co-op HUD, effects, display modes, and capture diagnostics.
#pragma once
#ifndef COOP_HUD_H
#define COOP_HUD_H
#include <string.h>
enum CoOpHudStyle {HUD_SHARED,HUD_CLUSTER,HUD_OVERLAY,HUD_PROJECTED,HUD_BOSS,
    HUD_COMPASS,HUD_RACE};
struct CoOpHudVertex {float x,y,u,v;unsigned long color;};
struct CoOpHudQuad {CoOpHudVertex v[4];float depth;};
struct CoOpHudRect {float x,y,w,h,u0,v0,u1,v1;unsigned long color;float depth;bool flag;};
struct CoOpHudPane {float x,y,w,h;};
struct CoOpHudCommand {
    CoOpHudQuad quad;
    CoOpHudRect rect;
    unsigned owner,style;
    bool rectangle,readable;
    float textLeft,textRight;
    bool text;
    int texture,blend;
};
// Prepare hud map for the owning viewport.
inline void CoOp_HudMap(CoOpHudQuad& q,unsigned owner,unsigned style,
    bool readable,bool text,float textLeft,float textRight,
    const CoOpHudPane& pane,float width,float height)
{
    if(!owner || style==HUD_SHARED || style==HUD_PROJECTED) return;
    float left=q.v[0].x,right=left,cx=0,cy=0;
    for(unsigned i=0;i<4;++i) {
        if(q.v[i].x<left) left=q.v[i].x;
        if(q.v[i].x>right) right=q.v[i].x;
        cx+=q.v[i].x*.25f;cy+=q.v[i].y*.25f;
    }
    if(text) {left=textLeft;right=textRight;}
    if(style==HUD_CLUSTER) {
        for(unsigned i=0;i<4;++i) {
            q.v[i].x=owner==2 ?
                pane.x+pane.w-(readable?left+right-q.v[i].x:q.v[i].x):
                pane.x+q.v[i].x;
            q.v[i].y+=pane.y;
        }
        if(owner==2 && !readable) {
            CoOpHudVertex t=q.v[0];q.v[0]=q.v[1];q.v[1]=t;
            t=q.v[2];q.v[2]=q.v[3];q.v[3]=t;
        }
    } else {
        if(text) cx=(textLeft+textRight)*.5f;
        float scale=pane.w/width;
        if(pane.h/height<scale) scale=pane.h/height;
        for(unsigned i=0;i<4;++i) {
            q.v[i].x=pane.x+cx*pane.w/width+(q.v[i].x-cx)*scale;
            q.v[i].y=pane.y+cy*pane.h/height+(q.v[i].y-cy)*scale;
        }
    }
}
// Prepare hud rect quad for the owning viewport.
inline CoOpHudQuad CoOp_HudRectQuad(const CoOpHudRect& r)
{
    CoOpHudQuad q;
    q.v[0].x=q.v[2].x=r.x;q.v[1].x=q.v[3].x=r.x+r.w;
    q.v[0].y=q.v[1].y=r.y;q.v[2].y=q.v[3].y=r.y+r.h;
    q.v[0].u=q.v[2].u=r.u0;q.v[1].u=q.v[3].u=r.u0+r.u1;
    q.v[0].v=q.v[1].v=r.v0;q.v[2].v=q.v[3].v=r.v0+r.v1;
    for(unsigned i=0;i<4;++i) q.v[i].color=r.color;
    q.depth=r.depth;return q;
}
template<unsigned Capacity=4096> class CoOpHudBuffer {
    CoOpHudCommand commands[Capacity];
    unsigned count,dropped;
public:
    // Initialize co-op hud buffer with no active retail ownership.
    CoOpHudBuffer():count(0),dropped(0) {}
    // Release owned resources and reset the owning viewport without changing the shared world to its inactive state.
    void Clear() {count=dropped=0;}
    // Register a retail-global byte range before the context is captured.
    void Add(const CoOpHudCommand& command) {
        if(count==Capacity) {++dropped;return;}
        commands[count++]=command;
    }
    // Return count from the owning viewport without changing the shared world.
    unsigned Count() const {return count;}
    // Return dropped from the owning viewport without changing the shared world.
    unsigned Dropped() const {return dropped;}
    // Return the current value owned by this module without changing retail state.
    const CoOpHudCommand& Get(unsigned i) const {return commands[i];}
};
// Move the retail boss bars as a unit. When a second protected character has
// a bar, separate its original row and place it immediately above the boss.
struct CoOpHudBossLayout {
    bool active,split;
    unsigned sourceCount;
    float centers[2],dx[2],dy[2];
    // Initialize co-op hud boss layout with no active retail ownership.
    CoOpHudBossLayout():active(false),split(false),sourceCount(0) {
        centers[0]=centers[1]=dx[0]=dx[1]=dy[0]=dy[1]=0;
    }
    // Prepare center y for the owning viewport.
    static float CenterY(const CoOpHudCommand& c) {
        float y=0;for(unsigned j=0;j<4;++j)y+=c.quad.v[j].y*.25f;return y;
    }
    // Prepare group for the owning viewport.
    unsigned Group(const CoOpHudCommand& c) const {
        return split && (CenterY(c)-centers[1])*(CenterY(c)-centers[1]) <
            (CenterY(c)-centers[0])*(CenterY(c)-centers[0]) ? 1:0;
    }
    template<unsigned Capacity> void Build(const CoOpHudBuffer<Capacity>& buffer,
        float width,float height,bool additional) {
        if(width<=0 || height<=0)return;
        float low=1e9f,high=-1e9f,first=0;
        unsigned count=0,i;
        for(i=0;i<buffer.Count();++i) {
            const CoOpHudCommand& c=buffer.Get(i);
            if(c.owner || c.style!=HUD_BOSS)continue;
            float y=CenterY(c);if(!count)first=y;
            if(y<low)low=y;if(y>high)high=y;++count;
        }
        if(!count)return;
        active=true;sourceCount=count;centers[0]=low;centers[1]=high;
        split=additional && count>2 && high-low>height*.025f;
        if(split)for(unsigned pass=0;pass<5;++pass) {
            float sum[2]={0,0};unsigned counts[2]={0,0};
            for(i=0;i<buffer.Count();++i) {
                const CoOpHudCommand& c=buffer.Get(i);
                if(c.owner || c.style!=HUD_BOSS)continue;
                unsigned g=Group(c);sum[g]+=CenterY(c);++counts[g];
            }
            if(!counts[0] || !counts[1]){split=false;break;}
            centers[0]=sum[0]/counts[0];centers[1]=sum[1]/counts[1];
        }
        float minX[2]={1e9f,1e9f},maxX[2]={-1e9f,-1e9f};
        float minY[2]={1e9f,1e9f},maxY[2]={-1e9f,-1e9f};
        for(i=0;i<buffer.Count();++i) {
            const CoOpHudCommand& c=buffer.Get(i);
            if(c.owner || c.style!=HUD_BOSS)continue;
            unsigned g=Group(c);
            for(unsigned j=0;j<4;++j) {
                float x=c.quad.v[j].x,y=c.quad.v[j].y;
                if(x<minX[g])minX[g]=x;if(x>maxX[g])maxX[g]=x;
                if(y<minY[g])minY[g]=y;if(y>maxY[g])maxY[g]=y;
            }
        }
        unsigned boss=split && (first-centers[1])*(first-centers[1]) <
            (first-centers[0])*(first-centers[0]) ? 1:0;
        dx[boss]=width*.5f-(minX[boss]+maxX[boss])*.5f;
        dy[boss]=height-height*.025f-maxY[boss];
        if(split) {
            unsigned extra=1-boss;
            dx[extra]=width*.5f-(minX[extra]+maxX[extra])*.5f;
            dy[extra]=minY[boss]+dy[boss]-height*.0125f-maxY[extra];
        }
    }
    // Prepare offset for the owning viewport.
    void Offset(const CoOpHudCommand& c,float& x,float& y) const {
        x=y=0;if(active && !c.owner && c.style==HUD_BOSS) {
            unsigned g=Group(c);x=dx[g];y=dy[g];
        }
    }
};
// Race callbacks draw several textured pieces and labels. Move the complete
// group together and test its final bounds against both mapped compasses.
struct CoOpHudRaceLayout {
    bool active;
    unsigned sourceCount;
    float dx,dy;
    // Initialize co-op hud race layout with no active retail ownership.
    CoOpHudRaceLayout():active(false),sourceCount(0),dx(0),dy(0) {}
    // Prepare overlap for the owning viewport.
    static bool Overlap(float left,float top,float right,float bottom,
        float otherLeft,float otherTop,float otherRight,float otherBottom,float gap) {
        return left<otherRight+gap && right>otherLeft-gap &&
            top<otherBottom+gap && bottom>otherTop-gap;
    }
    template<unsigned Capacity> bool HitsCompass(const CoOpHudBuffer<Capacity>& buffer,
        float left,float top,float right,float bottom,float gap,
        const CoOpHudPane panes[2],const bool visible[2],float width,float height,
        float* highestTop) const {
        bool hit=false;
        for(unsigned i=0;i<buffer.Count();++i) {
            const CoOpHudCommand& c=buffer.Get(i);
            if(c.style!=HUD_COMPASS || c.owner<1 || c.owner>2 || !visible[c.owner-1])continue;
            CoOpHudQuad q=c.quad;
            CoOp_HudMap(q,c.owner,c.style,c.readable,c.text,c.textLeft,c.textRight,
                panes[c.owner-1],width,height);
            float x0=q.v[0].x,x1=x0,y0=q.v[0].y,y1=y0;
            for(unsigned j=1;j<4;++j) {
                if(q.v[j].x<x0)x0=q.v[j].x;if(q.v[j].x>x1)x1=q.v[j].x;
                if(q.v[j].y<y0)y0=q.v[j].y;if(q.v[j].y>y1)y1=q.v[j].y;
            }
            if(Overlap(left,top,right,bottom,x0,y0,x1,y1,gap)) {
                hit=true;if(highestTop && y0<*highestTop)*highestTop=y0;
            }
        }
        return hit;
    }
    template<unsigned Capacity> void Build(const CoOpHudBuffer<Capacity>& buffer,
        float width,float height,const CoOpHudPane panes[2],const bool visible[2]) {
        if(width<=0 || height<=0)return;
        float x0=1e9f,y0=1e9f,x1=-1e9f,y1=-1e9f;
        for(unsigned i=0;i<buffer.Count();++i) {
            const CoOpHudCommand& c=buffer.Get(i);
            if(c.owner || c.style!=HUD_RACE)continue;
            ++sourceCount;
            for(unsigned j=0;j<4;++j) {
                float x=c.quad.v[j].x,y=c.quad.v[j].y;
                if(x<x0)x0=x;if(x>x1)x1=x;
                if(y<y0)y0=y;if(y>y1)y1=y;
            }
        }
        if(!sourceCount)return;
        active=true;
        float gap=height*.015f;if(gap<6)gap=6;
        dy=height-gap-y1;
        float centers[3]={width*.5f,gap+(x1-x0)*.5f,width-gap-(x1-x0)*.5f};
        for(unsigned n=0;n<3;++n) {
            float shift=centers[n]-(x0+x1)*.5f;
            if(!HitsCompass(buffer,x0+shift,y0+dy,x1+shift,y1+dy,gap,
                panes,visible,width,height,0)) {dx=shift;return;}
        }
        // If every bottom position is occupied, keep the bar centered and
        // lift it just enough to clear the compass artwork.
        dx=centers[0]-(x0+x1)*.5f;
        float top=height;
        HitsCompass(buffer,x0+dx,y0+dy,x1+dx,y1+dy,gap,
            panes,visible,width,height,&top);
        if(top<height)dy=top-gap-y1;
    }
    // Prepare offset for the owning viewport.
    void Offset(const CoOpHudCommand& c,float& x,float& y) const {
        x=y=0;if(active && !c.owner && c.style==HUD_RACE){x=dx;y=dy;}
    }
};
#endif
