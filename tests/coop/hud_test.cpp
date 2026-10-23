#include <assert.h>
#include <stdio.h>
#include "coop/presentation/coop_hud.h"
static CoOpHudQuad observed;
static void __cdecl Receive(CoOpHudQuad q) {observed=q;}
static CoOpHudRect observedRect;
static void __cdecl ReceiveRect(CoOpHudRect r) {observedRect=r;}
static CoOpHudQuad Quad(float x,float y,float w,float h) {
    CoOpHudRect r;memset(&r,0,sizeof(r));r.x=x;r.y=y;r.w=w;r.h=h;r.u1=r.v1=1;r.depth=-3;
    return CoOp_HudRectQuad(r);
}
int main() {
    assert(sizeof(CoOpHudQuad)==84 && sizeof(CoOpHudRect)==44);
    typedef void (__cdecl *RetailQuad)(float,float,float,float,unsigned long,float,float,float,float,unsigned long,float,float,float,float,unsigned long,float,float,float,float,unsigned long,float);
    reinterpret_cast<RetailQuad>(&Receive)(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21);
    assert(observed.v[0].x==1 && observed.v[1].x==6 && observed.v[3].color==20 && observed.depth==21);
    typedef void (__cdecl *RetailRect)(float,float,float,float,float,float,float,float,unsigned long,float,bool);
    reinterpret_cast<RetailRect>(&ReceiveRect)(1,2,3,4,5,6,7,8,9,10,true);
    assert(observedRect.x==1 && observedRect.color==9 && observedRect.depth==10 && observedRect.flag);
    CoOpHudPane panes[2]={{320,0,321,480},{0,240,640,241}};
    for(unsigned p=0;p<2;++p) {
        CoOpHudPane pane=panes[p];CoOpHudQuad q=Quad(20,30,40,10);
        CoOp_HudMap(q,2,HUD_CLUSTER,false,false,0,0,pane,640,480);
        assert(q.v[0].x==pane.x+pane.w-60 && q.v[1].x==pane.x+pane.w-20);
        assert(q.v[0].u==1 && q.v[1].u==0 && q.v[0].y==pane.y+30);
        q=Quad(20,30,40,10);CoOp_HudMap(q,2,HUD_CLUSTER,true,false,0,0,pane,640,480);
        assert(q.v[0].x==pane.x+pane.w-60 && q.v[0].u==0 && q.v[1].u==1);
        CoOpHudQuad first=Quad(20,30,8,10),second=Quad(30,30,8,10);
        CoOp_HudMap(first,2,HUD_CLUSTER,true,true,20,38,pane,640,480);
        CoOp_HudMap(second,2,HUD_CLUSTER,true,true,20,38,pane,640,480);
        assert(second.v[0].x-first.v[0].x==10);
        first=Quad(20,30,8,10);second=Quad(30,30,8,10);
        CoOp_HudMap(first,2,HUD_OVERLAY,true,true,20,38,pane,640,480);
        CoOp_HudMap(second,2,HUD_OVERLAY,true,true,20,38,pane,640,480);
        float scale=pane.w/640;if(pane.h/480<scale)scale=pane.h/480;
        float error=(second.v[0].x-first.v[0].x)-10*scale;assert(error<.001f && error>-.001f);
        q=Quad(20,30,40,10);CoOpHudQuad original=q;
        CoOp_HudMap(q,0,HUD_SHARED,false,false,0,0,pane,640,480);assert(!memcmp(&q,&original,sizeof(q)));
        CoOp_HudMap(q,2,HUD_PROJECTED,false,false,0,0,pane,640,480);assert(!memcmp(&q,&original,sizeof(q)));
        CoOp_HudMap(q,1,HUD_CLUSTER,false,false,0,0,pane,640,480);assert(q.v[0].x==pane.x+20 && q.v[0].y==pane.y+30);
    }
    CoOpHudRect atlas;memset(&atlas,0,sizeof(atlas));atlas.u0=.25f;atlas.v0=.5f;atlas.u1=.125f;atlas.v1=.25f;
    CoOpHudQuad atlasQuad=CoOp_HudRectQuad(atlas);
    assert(atlasQuad.v[1].u==.375f && atlasQuad.v[2].v==.75f);
    CoOpHudBuffer<2> buffer;CoOpHudCommand c;memset(&c,0,sizeof(c));c.owner=2;
    buffer.Add(c);buffer.Add(c);buffer.Add(c);assert(buffer.Count()==2 && buffer.Dropped()==1 && buffer.Get(1).owner==2);
    buffer.Clear();assert(!buffer.Count() && !buffer.Dropped());
    CoOpHudBuffer<8> bars;CoOpHudCommand bar;memset(&bar,0,sizeof(bar));
    bar.style=HUD_BOSS;bar.quad=Quad(100,400,200,25);bars.Add(bar);
    bar.quad=Quad(110,405,150,12);bars.Add(bar);
    bar.quad=Quad(700,100,180,20);bars.Add(bar);
    bar.quad=Quad(710,103,130,12);bars.Add(bar);
    CoOpHudBossLayout layout;layout.Build(bars,1280,720,true);
    float bx,by,ex,ey;layout.Offset(bars.Get(0),bx,by);
    layout.Offset(bars.Get(2),ex,ey);
    assert(layout.active && layout.split);
    assert(100+bx==540 && 700+ex==550); // each row centers on the full screen
    assert(425+by==702 && 120+ey==668); // extra row sits above the boss
    CoOpHudBossLayout single;single.Build(bars,1280,720,false);
    assert(single.active && !single.split);
    CoOpHudBuffer<8> race;CoOpHudCommand piece;memset(&piece,0,sizeof(piece));
    piece.style=HUD_RACE;piece.quad=Quad(80,20,140,20);race.Add(piece);
    piece.quad=Quad(100,24,100,10);race.Add(piece);
    CoOpHudPane racePanes[2]={{0,0,320,480},{320,0,320,480}};
    bool shown[2]={true,true};
    CoOpHudRaceLayout progress;progress.Build(race,640,480,racePanes,shown);
    assert(progress.active && progress.sourceCount==2);
    assert(80+progress.dx==250 && 40+progress.dy>472 && 40+progress.dy<474);
    // Both split orientations must avoid whichever personal compass is present.
    piece.style=HUD_COMPASS;piece.owner=1;piece.quad=Quad(500,440,100,30);race.Add(piece);
    CoOpHudRaceLayout vertical;vertical.Build(race,640,480,racePanes,shown);
    assert(40+vertical.dy>472 && 40+vertical.dy<474 && 80+vertical.dx<100);
    assert(!vertical.HitsCompass(race,80+vertical.dx,20+vertical.dy,
        220+vertical.dx,40+vertical.dy,7,racePanes,shown,640,480,0));
    racePanes[0].w=racePanes[1].w=640;
    racePanes[0].h=racePanes[1].h=240;
    racePanes[1].x=0;racePanes[1].y=240;
    piece.owner=2;piece.quad=Quad(250,430,140,40);race.Add(piece);
    CoOpHudRaceLayout horizontal;horizontal.Build(race,640,480,racePanes,shown);
    assert(horizontal.active && 40+horizontal.dy>472 && 40+horizontal.dy<474);
    assert(!horizontal.HitsCompass(race,80+horizontal.dx,20+horizontal.dy,
        220+horizontal.dx,40+horizontal.dy,7,racePanes,shown,640,480,0));
    race.Clear();piece.owner=0;piece.style=HUD_RACE;piece.quad=Quad(80,20,140,20);race.Add(piece);
    piece.owner=1;piece.style=HUD_COMPASS;piece.quad=Quad(0,440,640,40);race.Add(piece);
    racePanes[0].w=640;racePanes[0].h=480;racePanes[0].y=0;
    CoOpHudRaceLayout lifted;lifted.Build(race,640,480,racePanes,shown);
    assert(lifted.active && 40+lifted.dy<440);
    assert(!lifted.HitsCompass(race,80+lifted.dx,20+lifted.dy,
        220+lifted.dx,40+lifted.dy,7,racePanes,shown,640,480,0));
    puts("PASS: native retail HUD argument ABI, mirrored artwork, readable icon/text, both layouts, shared/projected coordinates and bounded queue");
    return 0;
}
