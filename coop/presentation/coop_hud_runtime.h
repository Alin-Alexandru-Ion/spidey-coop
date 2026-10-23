// Co-op / presentation: Render co-op HUD, effects, display modes, and capture diagnostics.
#pragma once
#ifndef COOP_HUD_RUNTIME_H
#define COOP_HUD_RUNTIME_H
#include "coop_hud.h"
#include "coop_partner_marker.h"
#include "mess.h"
static CoOpHudBuffer<> gHudBuffer;
struct CoOpHudCapture {
    bool active,readable,text;
    unsigned owner,style;
    float left,right;
};
static CoOpHudCapture gHudCapture;
static bool gHudInstalled,gHudPanel,gHudFade,gHudTarget,gHudArrow;
static bool gHudDrawingPanel;
static unsigned gHudLastCount,gHudLastDropped,gHudPanelCount[2],gHudCompassCount[2],gHudTimerCount;
static unsigned gHudPartnerState[2]; // 0 hidden, 1 label, 2 edge arrow
static CoOpHudBossLayout gHudBossLast;
static CoOpHudRaceLayout gHudRaceLast;
static void* gHudQuadOriginal;
static void* gHudRectOriginal;
static void* gHudSimpleOriginal;
typedef void (__cdecl *CoOpHudQuadFn)(CoOpHudQuad);
typedef void (__cdecl *CoOpHudRectFn)(CoOpHudRect);
// Draw the co-op HUD only while a playable or waiting P2 viewport exists.
static bool CoOp_HudEnabled() {
    return gHudInstalled && (!*reinterpret_cast<i32*>(0x00660F80) || CoOp_LifeWaiting(0) || CoOp_LifeWaiting(1)) &&
        gCoOpPlayers.GetRenderBody(COOP_PLAYER_TWO) &&
        *reinterpret_cast<void**>(0x006B791C) && !*reinterpret_cast<i32*>(0x006B78F8);
}
class CoOpHudScope {
    CoOpHudCapture saved;
public:
    // Initialize co-op hud scope with no active retail ownership.
    CoOpHudScope(unsigned owner,unsigned style):saved(gHudCapture) {
        memset(&gHudCapture,0,sizeof(gHudCapture));
        gHudCapture.active=true;gHudCapture.owner=owner;gHudCapture.style=style;
    }
    // Release the scoped state owned by this object.
    ~CoOpHudScope() {gHudCapture=saved;}
};
// Clear captured HUD commands and per-pane counters before retail rendering.
static void CoOp_HudBeginFrame() {
    gHudBuffer.Clear();memset(&gHudCapture,0,sizeof(gHudCapture));
    gHudRaceLast=CoOpHudRaceLayout();
    gHudPanel=gHudFade=gHudTarget=gHudArrow=false;
    gHudPanelCount[0]=gHudPanelCount[1]=gHudCompassCount[0]=gHudCompassCount[1]=gHudTimerCount=0;
    gHudPartnerState[0]=gHudPartnerState[1]=0;
}
// Snapshot retail texture and blend state for one queued HUD command.
static CoOpHudCommand CoOp_HudCommand() {
    CoOpHudCommand c;memset(&c,0,sizeof(c));
    c.owner=gHudCapture.owner;c.style=gHudCapture.style;
    c.readable=gHudCapture.readable;c.text=gHudCapture.text;
    c.textLeft=gHudCapture.left;c.textRight=gHudCapture.right;
    c.texture=*reinterpret_cast<int*>(0x00568170);
    c.blend=*reinterpret_cast<int*>(0x00AC08DC);return c;
}
// Offset enlarged head art so its edges meet the original bar geometry.
static void CoOp_HudAlignRemadeHead(CoOpHudQuad& q) {
    if(!gHudDrawingPanel ||
        !CoOpUiRemakeIsHeadTexture(*reinterpret_cast<int*>(0x00568170))) return;
    float minX=q.v[0].x,maxX=minX,minY=q.v[0].y,maxY=minY;
    for(unsigned i=1;i<4;++i) {
        if(q.v[i].x<minX)minX=q.v[i].x;if(q.v[i].x>maxX)maxX=q.v[i].x;
        if(q.v[i].y<minY)minY=q.v[i].y;if(q.v[i].y>maxY)maxY=q.v[i].y;
    }
    // The 8x head art starts five retail pixels early on both joining edges.
    const float dx=(maxX-minX)*(5.0f/64.0f),dy=(maxY-minY)*(5.0f/64.0f);
    for(unsigned j=0;j<4;++j) {q.v[j].x+=dx;q.v[j].y+=dy;}
}
// Prepare hud quad for the owning viewport.
static void __cdecl CoOp_HudQuad(CoOpHudQuad q) {
    CoOp_HudAlignRemadeHead(q);
    if(!gHudCapture.active) {reinterpret_cast<CoOpHudQuadFn>(gHudQuadOriginal)(q);return;}
    CoOpHudCommand c=CoOp_HudCommand();c.quad=q;gHudBuffer.Add(c);
}
// Prepare hud rect for the owning viewport.
static void __cdecl CoOp_HudRect(CoOpHudRect r) {
    if(gHudDrawingPanel && CoOpUiRemakeIsHeadTexture(*reinterpret_cast<int*>(0x00568170))) {
        r.x+=r.w*(5.0f/64.0f);r.y+=r.h*(5.0f/64.0f);
    }
    if(!gHudCapture.active) {reinterpret_cast<CoOpHudRectFn>(gHudRectOriginal)(r);return;}
    CoOpHudCommand c=CoOp_HudCommand();c.rectangle=true;c.rect=r;
    c.quad=CoOp_HudRectQuad(r);gHudBuffer.Add(c);
}
// Prepare hud web icon for the owning viewport.
static void __cdecl CoOp_HudWebIcon(CoOpHudQuad q) {
    bool saved=gHudCapture.readable;gHudCapture.readable=true;
    CoOp_HudQuad(q);gHudCapture.readable=saved;
}
// Prepare hud text for the owning viewport.
static i32 CoOp_HudText(i32 x,i32 y,const char* text,i32 a,u32 b) {
    typedef i32 (__cdecl *TextFn)(i32,i32,const char*,i32,u32);
    typedef i32 (__cdecl *WidthFn)(const char*);
    CoOpHudCapture saved=gHudCapture;
    if(gHudCapture.active) {
        float sx=(float)*reinterpret_cast<i32*>(0x00568154)/
            *reinterpret_cast<u32*>(0x0061B5FC);
        gHudCapture.readable=gHudCapture.text=true;
        float textWidth=(float)reinterpret_cast<WidthFn>(0x004586C0)(text);
        unsigned justify=*reinterpret_cast<u8*>(0x0060D5A8);
        float start=(float)x;
        if(justify==0) start-=textWidth*.5f;
        else if(justify==2) start-=textWidth;
        gHudCapture.left=start*sx;
        gHudCapture.right=(start+textWidth)*sx;
    }
    i32 result=reinterpret_cast<TextFn>(0x00458700)(x,y,text,a,b);
    gHudCapture=saved;return result;
}
// Mark a native text draw as shared so it appears only once across split views.
static i32 __cdecl CoOp_HudSharedText(i32 x,i32 y,const char* text,i32 a,u32 b) {
    if(!CoOp_HudEnabled()) return CoOp_HudText(x,y,text,a,b);
    CoOpHudScope scope(0,HUD_SHARED);return CoOp_HudText(x,y,text,a,b);
}
// Pass retail ammo text through the active player HUD capture.
static i32 __cdecl CoOp_HudAmmo(i32 x,i32 y,const char* text,i32 a,u32 b) {
    return CoOp_HudText(x,y,text,a,b);
}
// Draw the native timer once as shared presentation, even with two panels.
static void __cdecl CoOp_HudTimer() {
    if(gHudCapture.active && gHudCapture.owner==2) return;
    typedef void (__cdecl *Fn)();
    if(gHudCapture.active) {
        CoOpHudScope scope(0,HUD_SHARED);++gHudTimerCount;
        reinterpret_cast<Fn>(0x00461D00)();
    } else reinterpret_cast<Fn>(0x00461D00)();
}
// Place the shared race indicator below the split views.
static void CoOp_HudRaceProgress(u32 original,const u32* input,u32* output) {
    typedef void (__cdecl *Fn)(const u32*,u32*);
    if(CoOp_HudEnabled() && gHudCapture.active) {
        CoOpHudScope scope(0,HUD_RACE);
        reinterpret_cast<Fn>(original)(input,output);
    } else reinterpret_cast<Fn>(original)(input,output);
}
// Draw the native boss bar once in a common split-screen region.
static void __cdecl CoOp_HudBoss() {
    if(gHudCapture.active && gHudCapture.owner==2) return;
    typedef void (__cdecl *Fn)();
    if(gHudCapture.active) {
        CoOpHudScope scope(0,HUD_BOSS);reinterpret_cast<Fn>(0x00464270)();
    } else reinterpret_cast<Fn>(0x00464270)();
}
// Draw each player's compass from that player's camera and objective state.
static void __cdecl CoOp_HudCompass() {
    typedef void (__cdecl *Fn)();
    // Panel_Display invokes this through two alternate branches. Capture it
    // explicitly once for each participant after its panel has been drawn;
    // those branches can skip a live player's compass in co-op.
    if(CoOp_HudEnabled() && gHudCapture.active)return;
    reinterpret_cast<Fn>(0x00463860)();
}
// Invoke native compass drawing inside this player's HUD and camera context.
static void CoOp_HudPersonalCompass(unsigned owner) {
    if(!*reinterpret_cast<u8*>(0x0060F77C))return;
    CoOpHudScope scope(owner,HUD_COMPASS);
    reinterpret_cast<void (__cdecl*)()>(0x00463860)();
    ++gHudCompassCount[owner-1];
}
// Capture P1's native panel, then draw its compass once.
static void __cdecl CoOp_HudPanel() {
    typedef void (__cdecl *Fn)();
    bool saved=gHudDrawingPanel;gHudDrawingPanel=true;
    if(!CoOp_HudEnabled()) {
        reinterpret_cast<Fn>(0x004658C0)();gHudDrawingPanel=saved;return;
    }
    gHudPanel=true;++gHudPanelCount[0];
    CoOpHudScope scope(1,HUD_CLUSTER);
    reinterpret_cast<Fn>(0x004658C0)(); // original timer/update executes once
    CoOp_HudPersonalCompass(1);
    gHudDrawingPanel=saved;
}
struct CoOpHudAssets {u32 sprite[5];u8 target;};
// Save the retail HUD sprites and targeting flag before switching owner.
static CoOpHudAssets CoOp_HudAssets() {
    CoOpHudAssets a;
    memcpy(a.sprite,reinterpret_cast<void*>(0x0060F750),sizeof(a.sprite));
    a.target=*reinterpret_cast<u8*>(0x006A7572);return a;
}
// Prepare hud supply assets for the owning viewport.
static void CoOp_HudSupplyAssets(const CoOpHudAssets& a) {
    // Sp, Compass and Webcart are level-owned shared sprites, not player effects.
    memcpy(reinterpret_cast<void*>(0x0060F758),a.sprite+2,3*sizeof(u32));
    if(!*reinterpret_cast<u32*>(0x0060F750))
        *reinterpret_cast<u32*>(0x0060F750)=a.sprite[0]; // same starting costume
}
// Prepare hud second panel for the owning viewport.
static void CoOp_HudSecondPanel(void* argument) {
    CoOp_HudSupplyAssets(*static_cast<CoOpHudAssets*>(argument));
    CoOpHudScope scope(2,HUD_CLUSTER);
    u8 savedTimer[12];memcpy(savedTimer,reinterpret_cast<void*>(0x0060F774),12);
    u8 timerEnabled=*reinterpret_cast<u8*>(0x0060F772);
    *reinterpret_cast<u8*>(0x0060F772)=0; // suppress inline shared countdown update
    typedef void (__cdecl *Fn)();
    bool savedDrawingPanel=gHudDrawingPanel;gHudDrawingPanel=true;
    ++gHudPanelCount[1];reinterpret_cast<Fn>(0x004658C0)();
    CoOp_HudPersonalCompass(2);
    gHudDrawingPanel=savedDrawingPanel;
    *reinterpret_cast<u8*>(0x0060F772)=timerEnabled;
    memcpy(reinterpret_cast<void*>(0x0060F774),savedTimer,12);
}
static void __fastcall CoOp_HudSense(CPlayer* player,void*) {
    typedef void (__fastcall *Fn)(CPlayer*,void*);
    if(!CoOp_HudEnabled()) {reinterpret_cast<Fn>(0x004C54A0)(player,0);return;}
    unsigned owner=gCoOpPlayers.InSecondaryContext()?2:1;
    CoOpHudScope scope(owner,HUD_OVERLAY);
    // This method ages indicator trails. Capture its original AI-time call;
    // never run it again for an additional viewport or while paused.
    reinterpret_cast<Fn>(0x004C54A0)(player,0);
}
static void __fastcall CoOp_HudReticle(CPlayer* player,void*) {
    typedef void (__fastcall *Fn)(CPlayer*,void*);
    if(!CoOp_HudEnabled()) {reinterpret_cast<Fn>(0x004C4940)(player,0);return;}
    CoOpHudScope scope(1,HUD_PROJECTED);
    CoOp_DrawProjectedPlayer(player,gCoOpEffectsCamera,reinterpret_cast<void (__fastcall*)(void*,void*)>(0x004C4940));
}
// Prepare hud target for the owning viewport.
static void __cdecl CoOp_HudTarget() {
    if(CoOp_HudEnabled()) {gHudTarget=true;return;}
    reinterpret_cast<void (__cdecl*)()>(0x0048AA90)();
}
// Prepare hud arrow for the owning viewport.
static void __cdecl CoOp_HudArrow() {
    if(CoOp_HudEnabled()) {gHudArrow=true;return;}
    reinterpret_cast<void (__cdecl*)()>(0x0048AE30)();
}
// Prepare hud view markers for the owning viewport.
static void CoOp_HudViewMarkers(unsigned owner) {
    if(!CoOp_HudEnabled()) return;
    CoOpHudScope scope(owner,HUD_PROJECTED);
    if(gHudTarget || owner==2) reinterpret_cast<void (__cdecl*)()>(0x0048AA90)();
    if(gHudArrow) reinterpret_cast<void (__cdecl*)()>(0x0048AE30)();
}
// Prepare hud second view for the owning viewport.
static void CoOp_HudSecondView(void* argument) {
    CoOpHudAssets* assets=static_cast<CoOpHudAssets*>(argument);
    CoOp_HudSupplyAssets(*assets);
    // Keep the native target flag and position from P2's own gameplay bank.
    CoOpHudScope scope(2,HUD_PROJECTED);
    typedef void (__fastcall *Fn)(CPlayer*,void*);
    CoOp_DrawProjectedPlayer(gCoOpPlayers.Get(COOP_PLAYER_TWO),gCoOpSecondCamera,
        reinterpret_cast<void (__fastcall*)(void*,void*)>(0x004C4940));
    CoOp_HudViewMarkers(2);
}
// Capture hud second view for the owning viewport without changing the shared world.
static void CoOp_HudCaptureSecondView() {
    if(!CoOp_HudEnabled()) return;
    CoOpHudAssets a=CoOp_HudAssets();
    gCoOpPlayers.WithSecondaryContext(CoOp_HudSecondView,&a);
}
// Prepare hud fade for the owning viewport.
static void __cdecl CoOp_HudFade() {
    if(CoOp_HudEnabled()) {gHudFade=true;return;}
    reinterpret_cast<void (__cdecl*)()>(0x0048AFE0)();
}
// Prepare hud training for the owning viewport.
static void __cdecl CoOp_HudTraining() {
    typedef void (__cdecl *Fn)();
    if(!CoOp_HudEnabled()) {reinterpret_cast<Fn>(0x0047BA50)();return;}
    CoOpHudScope scope(0,HUD_SHARED);reinterpret_cast<Fn>(0x0047BA50)();
}
struct CoOpHudMessageOwner {SSimpleMessage* message;CoOpPlayerHandle owner;};
static CoOpHudMessageOwner gHudMessages[128];
static unsigned gHudMessageNext;
// Prepare hud simple message for the owning viewport.
static SSimpleMessage* __cdecl CoOp_HudSimpleMessage(const char* text,u32 a,u32 b,u32 c) {
    typedef SSimpleMessage* (__cdecl *Fn)(const char*,u32,u32,u32);
    SSimpleMessage* result=reinterpret_cast<Fn>(gHudSimpleOriginal)(text,a,b,c);
    if(!result) return result;
    unsigned slot=COOP_PLAYER_COUNT;
    if(gCoOpPlayers.InSecondaryContext()) slot=COOP_PLAYER_TWO;
    else if(gCoOpTriggerOwner) slot=COOP_PLAYER_ONE;
    unsigned entry=128;
    for(unsigned i=0;i<128;++i) if(gHudMessages[i].message==result) {entry=i;break;}
    if(entry==128) entry=(gHudMessageNext++)%128;
    gHudMessages[entry].message=result;
    gHudMessages[entry].owner=gCoOpPlayers.Router().Handle((CoOpPlayerSlot)slot);
    return result;
}
// Prepare hud messages for the owning viewport.
static void __cdecl CoOp_HudMessages() {
    if(!CoOp_HudEnabled()) {reinterpret_cast<void (__cdecl*)()>(0x00458A40)();return;}
    // Same live simple-message list and styling; message lifetime updates remain
    // entirely in retail Mess_Update. No stored historic pointer is dereferenced.
    reinterpret_cast<void (__cdecl*)(u8)>(0x00458610)(1);
    reinterpret_cast<void (__cdecl*)(u8,u8,u8,i32)>(0x00458640)(0x60,0x60,0x80,0);
    for(SSimpleMessage* m=*reinterpret_cast<SSimpleMessage**>(0x0060D5A0);m;m=m->pNext) {
        unsigned owner=0;bool stale=false;
        for(unsigned i=0;i<128;++i) if(gHudMessages[i].message==m) {
            CoOpPlayerHandle h=gHudMessages[i].owner;
            if(h.slot<COOP_PLAYER_COUNT) {
                stale=!gCoOpPlayers.Router().Resolve(h);owner=h.slot+1;
            }
            break;
        }
        if(stale) continue;
        CoOpHudScope scope(owner,owner?HUD_OVERLAY:HUD_SHARED);
        reinterpret_cast<void (__cdecl*)(i32)>(0x00458620)(m->field_14);
        CoOp_HudText(m->field_C,m->field_10,m->field_0,0,m->field_18);
    }
}
// Prepare hud resume for the owning viewport.
static bool CoOp_HudResume(LPDIRECT3DDEVICE7 device) {
    if(FAILED(device->BeginScene())) return false;
    memset(reinterpret_cast<void*>(0x006B7AA0),0,0x1001*sizeof(u32));
    *reinterpret_cast<u8*>(0x006BBAA4)=1;*reinterpret_cast<u8*>(0x00AC08C4)=1;
    *reinterpret_cast<float*>(0x0073C77C)=0.01f;
    *reinterpret_cast<float*>(0x00AC07B8)=-0.2f;return true;
}
// Prepare hud replay command for the owning viewport.
static void CoOp_HudReplayCommand(const CoOpHudCommand& command,const CoOpHudPane& pane,
    float width,float height,const CoOpHudBossLayout& bossLayout,
    const CoOpHudRaceLayout& raceLayout) {
    CoOpHudQuad q=command.quad;
    CoOp_HudMap(q,command.owner,command.style,command.readable,command.text,
        command.textLeft,command.textRight,pane,width,height);
    float bossX,bossY,raceX,raceY;
    bossLayout.Offset(command,bossX,bossY);
    raceLayout.Offset(command,raceX,raceY);
    for(unsigned j=0;j<4;++j){q.v[j].x+=bossX+raceX;q.v[j].y+=bossY+raceY;}
    reinterpret_cast<void (__cdecl*)(i32,i32)>(0x00506440)(command.texture,command.blend);
    if(!command.rectangle) reinterpret_cast<CoOpHudQuadFn>(gHudQuadOriginal)(q);
    else {
        CoOpHudRect r=command.rect;
        r.x=q.v[0].x;r.y=q.v[0].y;r.w=q.v[1].x-r.x;r.h=q.v[2].y-r.y;
        r.u0=q.v[0].u;r.v0=q.v[0].v;r.u1=q.v[1].u-r.u0;r.v1=q.v[2].v-r.v0;
        reinterpret_cast<CoOpHudRectFn>(gHudRectOriginal)(r);
    }
}
// Clear only the dead player's viewport and draw its checkpoint message.
static void CoOp_HudWaitingPane(LPDIRECT3DDEVICE7 device,const CoOpHudPane& pane,unsigned owner,float width,float height) {
    device->Clear(0,0,D3DCLEAR_TARGET,0xFF000000,1.0f,0);
    Font* font=reinterpret_cast<Font*>(0x0060D238);Font** fonts=reinterpret_cast<Font**>(0x005FAD5C);
    Font* face=0;for(int i=0;i<6;++i)if(fonts[i] && !strcmp(fonts[i]->field_38,"sp_fnt02.fnt"))face=fonts[i];
    if(!face || !face->pCharTab)return;
    u8 savedFont[sizeof(Font)],savedText[8];memcpy(savedFont,font,sizeof(Font));
    memcpy(savedText,reinterpret_cast<void*>(0x0060D5A4),8);memcpy(font,face,sizeof(Font));
    font->mRed=font->mGreen=font->mBlue=255;
    *reinterpret_cast<u16*>(0x0060D5A4)=192;*reinterpret_cast<u16*>(0x0060D5A6)=4095;
    *reinterpret_cast<u8*>(0x0060D5A8)=0;
    typedef i32 (__cdecl *Fn)(i32,i32,const char*,i32,u32);Fn draw=reinterpret_cast<Fn>(0x00458700);
    int x=(int)((pane.x+pane.w*.5f)*(*reinterpret_cast<u32*>(0x0061B5FC))/width);
    int y=(int)((pane.y+pane.h*.5f)*(*reinterpret_cast<u32*>(0x00628614))/height);
    draw(x,y-12,owner==1?"P1":"P2",0,0x1000);
    draw(x,y+2,"Waiting for checkpoint respawn",0,0x1000);
    memcpy(font,savedFont,sizeof(Font));memcpy(reinterpret_cast<void*>(0x0060D5A4),savedText,8);
}
// Prepare hud viewport for the owning viewport.
static D3DVIEWPORT7 CoOp_HudViewport(const D3DVIEWPORT7& full,unsigned owner) {
    D3DVIEWPORT7 viewport=full;
    if(gCoOpLayout==COOP_SPLIT_LEFT_RIGHT) {
        viewport.dwWidth=full.dwWidth/2;
        if(owner==2) {viewport.dwX+=viewport.dwWidth;viewport.dwWidth=full.dwWidth-viewport.dwWidth;}
    } else if(gCoOpLayout==COOP_SPLIT_TOP_BOTTOM) {
        viewport.dwHeight=full.dwHeight/2;
        if(owner==2) {viewport.dwY+=viewport.dwHeight;viewport.dwHeight=full.dwHeight-viewport.dwHeight;}
    }
    return viewport;
}
// Prepare hud marker camera for the owning viewport.
static bool CoOp_HudMarkerCamera(unsigned owner,CoOpMarkerCamera& out) {
    if(owner<1 || owner>2 || !gCoOpMarkerProjectionValid[owner-1])return false;
    const SCamera* frame=reinterpret_cast<const SCamera*>(
        owner==1?gCoOpPrimaryCamera:gCoOpSecondCamera);
    out.position[0]=frame->Position.vx;
    out.position[1]=frame->Position.vy;
    out.position[2]=frame->Position.vz;
    for(unsigned row=0;row<3;++row)for(unsigned col=0;col<3;++col)
        out.rotation[row][col]=frame->Transform.m[row][col];
    const float* projection=gCoOpMarkerProjection[owner-1];
    out.focalX=projection[0];out.focalY=projection[5];
    out.centerX=projection[8];out.centerY=projection[9];
    return out.focalX>0 && out.focalY>0;
}
// Draw P1/P2 labels and arrows in their respective viewports.
static void CoOp_HudPartnerMarkers(const D3DVIEWPORT7& full) {
    for(unsigned owner=1;owner<=2;++owner) {
        if(!CoOp_ViewShowsOwner(gCoOpLayout,owner) ||
            CoOp_LifeWaiting(owner-1) || CoOp_LifeWaiting(2-owner))continue;
        if(owner==2 && !gCoOpSecondRendered)continue;
        CPlayer* viewer=gCoOpPlayers.Get((CoOpPlayerSlot)(owner-1));
        CPlayer* partner=gCoOpPlayers.Get((CoOpPlayerSlot)(2-owner));
        if(!viewer || !partner || viewer->mHealth<=0 || partner->mHealth<=0)continue;
        CoOpMarkerCamera camera;
        if(!CoOp_HudMarkerCamera(owner,camera))continue;
        D3DVIEWPORT7 viewport=CoOp_HudViewport(full,owner);
        CoOpHudPane pane={(float)viewport.dwX,(float)viewport.dwY,
            (float)viewport.dwWidth,(float)viewport.dwHeight};
        double origin[3]={partner->mPos.vx/4096.0,
            partner->mPos.vy/4096.0,partner->mPos.vz/4096.0};
        double target[3];
        CoOp_PartnerHeadTarget(origin,partner->mTransform.m,target);
        CoOpPartnerMarker marker=CoOp_ProjectPartner(camera,target,pane,(float)full.dwHeight);
        if(marker.valid)gHudPartnerState[owner-1]=marker.onScreen?1:2;
        CoOp_QueuePartnerMarker(gHudBuffer,marker,pane,owner);
    }
}
// Prepare hud finish for the owning viewport.
static void CoOp_HudFinish() {
    if(!CoOp_HudEnabled()) {gHudBuffer.Clear();return;}
    // P2 draw changes font state but must not affect the shared menu/message font.
    u8 font[sizeof(Font)];memcpy(font,reinterpret_cast<void*>(0x0060D238),sizeof(font));
    u8 textState[8];memcpy(textState,reinterpret_cast<void*>(0x0060D5A4),8);
    u8 gte[0x100];memcpy(gte,reinterpret_cast<void*>(0x00610B20),sizeof(gte));
    if(gHudPanel) {
        CoOpHudAssets a=CoOp_HudAssets();
        gCoOpPlayers.WithSecondaryContext(CoOp_HudSecondPanel,&a);
    }
    memcpy(reinterpret_cast<void*>(0x0060D238),font,sizeof(font));
    memcpy(reinterpret_cast<void*>(0x0060D5A4),textState,8);
    memcpy(reinterpret_cast<void*>(0x00610B20),gte,sizeof(gte));
    LPDIRECT3DDEVICE7 device=*reinterpret_cast<LPDIRECT3DDEVICE7*>(0x006B791C);
    D3DVIEWPORT7 full;if(!device || FAILED(device->GetViewport(&full))) {gHudBuffer.Clear();return;}
    CoOp_HudPartnerMarkers(full);
    gHudLastCount=gHudBuffer.Count();gHudLastDropped=gHudBuffer.Dropped();
    float width=(float)*reinterpret_cast<i32*>(0x00568154);
    float height=(float)*reinterpret_cast<i32*>(0x00568158);
    CoOpHudBossLayout bossLayout;
    bossLayout.Build(gHudBuffer,width,height,*reinterpret_cast<void**>(0x0060F78C)!=0);
    gHudBossLast=bossLayout;
    CoOpHudPane compassPanes[2];bool compassVisible[2];
    for(unsigned owner=1;owner<=2;++owner) {
        compassVisible[owner-1]=CoOp_ViewShowsOwner(gCoOpLayout,owner);
        D3DVIEWPORT7 viewport=CoOp_HudViewport(full,owner);
        compassPanes[owner-1].x=(float)viewport.dwX;
        compassPanes[owner-1].y=(float)viewport.dwY;
        compassPanes[owner-1].w=(float)viewport.dwWidth;
        compassPanes[owner-1].h=(float)viewport.dwHeight;
    }
    CoOpHudRaceLayout raceLayout;
    raceLayout.Build(gHudBuffer,width,height,compassPanes,compassVisible);
    gHudRaceLast=raceLayout;
    int texture=*reinterpret_cast<int*>(0x00568170),blend=*reinterpret_cast<int*>(0x00AC08DC);
    for(unsigned pass=0;pass<3;++pass) {
        unsigned owner=pass==2?0:pass+1;
        if(!CoOp_ViewShowsOwner(gCoOpLayout,owner)) continue;
        D3DVIEWPORT7 viewport=owner?CoOp_HudViewport(full,owner):full;
        device->SetViewport(&viewport);
        CoOpHudPane pane={(float)viewport.dwX,(float)viewport.dwY,
            (float)viewport.dwWidth,(float)viewport.dwHeight};
        bool waiting=owner && CoOp_LifeWaiting(owner-1);
        if(waiting) CoOp_HudWaitingPane(device,pane,owner,width,height);
        for(unsigned i=0;i<gHudBuffer.Count();++i)
            if(!waiting && gHudBuffer.Get(i).owner==owner)
                CoOp_HudReplayCommand(gHudBuffer.Get(i),pane,width,height,bossLayout,raceLayout);
        if(owner) {
            reinterpret_cast<void (__cdecl*)(i32)>(0x00506160)(0);
            if(!CoOp_HudResume(device)) break;
        }
    }
    device->SetViewport(&full);
    reinterpret_cast<void (__cdecl*)(i32,i32)>(0x00506440)(texture,blend);
    // Shared transition update/draw runs once, on top of both personal HUDs.
    if(gHudFade) reinterpret_cast<void (__cdecl*)()>(0x0048AFE0)();
    gHudBuffer.Clear();
}
// Install install hud hooks at the supported retail call site, preserving its fallback.
static bool CoOp_InstallHudHooks() {
    if(sizeof(CoOpHudQuad)!=84 || sizeof(CoOpHudRect)!=44) return false;
    const u32 sites[]={0x0045561A,0x00465960,0x00466A6D,0x00465988,0x00466A80,
        0x00465A83,0x00465D08,0x004B867A,0x00455792,0x00455615,
        0x004556EF,0x004556F4,0x004556F9,0x00455707,0x004555DE};
    const u32 targets[]={0x004658C0,0x00461D00,0x00464270,0x00463860,0x00463860,
        0x00458700,0x00507910,0x004C54A0,0x004C4940,0x0048AFE0,
        0x0048AA90,0x0048AE30,0x00458A40,0x0047BA50,0x00458700};
    for(unsigned i=0;i<sizeof(sites)/sizeof(sites[0]);++i) {
        const u8* code=reinterpret_cast<const u8*>(sites[i]);
        if(code[0]!=0xE8 || sites[i]+5+*reinterpret_cast<const i32*>(code+1)!=targets[i]) return false;
    }
    const u8 quadBytes[]={0xD9,0x44,0x24,0x54,0xA1,0x78,0x81,0x56,0x00};
    const u8 rectBytes[]={0xD9,0x44,0x24,0x28,0xA1,0x78,0x81,0x56,0x00};
    const u8 simpleBytes[]={0x56,0x57,0x33,0xFF,0x6A,0x01};
    if(memcmp(reinterpret_cast<void*>(0x00507910),quadBytes,9) ||
       memcmp(reinterpret_cast<void*>(0x00507470),rectBytes,9) ||
       memcmp(reinterpret_cast<void*>(0x00458850),simpleBytes,6)) return false;
    u8* tramp=static_cast<u8*>(VirtualAlloc(0,64,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE));
    if(!tramp) return false;
    const u32 starts[]={0x00507910,0x00507470,0x00458850};
    const unsigned lengths[]={9,9,6};
    for(unsigned t=0;t<3;++t) {
        u8* dest=tramp+t*16;unsigned n=lengths[t];
        memcpy(dest,reinterpret_cast<void*>(starts[t]),n);
        dest[n]=0xE9;*reinterpret_cast<i32*>(dest+n+1)=(i32)(starts[t]+n-(u32)(dest+n+5));
    }
    DWORD oldProtect;
    if(!VirtualProtect(reinterpret_cast<void*>(0x00401000),0x13A000,PAGE_EXECUTE_READWRITE,&oldProtect)) {
        VirtualFree(tramp,0,MEM_RELEASE);return false;
    }
    gHudQuadOriginal=tramp;gHudRectOriginal=tramp+16;gHudSimpleOriginal=tramp+32;
    PATCH_PUSH_RET(0x00507910,CoOp_HudQuad);
    PATCH_PUSH_RET(0x00507470,CoOp_HudRect);
    PATCH_PUSH_RET(0x00458850,CoOp_HudSimpleMessage);
    PATCH_CALL(0x0045561A,CoOp_HudPanel);
    PATCH_CALL(0x00465960,CoOp_HudTimer);
    PATCH_CALL(0x00466A6D,CoOp_HudBoss);
    PATCH_CALL(0x00465988,CoOp_HudCompass);
    PATCH_CALL(0x00466A80,CoOp_HudCompass);
    PATCH_CALL(0x00465A83,CoOp_HudAmmo);
    PATCH_CALL(0x00465D08,CoOp_HudWebIcon);
    PATCH_CALL(0x004B867A,CoOp_HudSense);
    PATCH_CALL(0x00455792,CoOp_HudReticle);
    PATCH_CALL(0x00455615,CoOp_HudFade);
    PATCH_CALL(0x004556EF,CoOp_HudTarget);
    PATCH_CALL(0x004556F4,CoOp_HudArrow);
    PATCH_CALL(0x004556F9,CoOp_HudMessages);
    PATCH_CALL(0x00455707,CoOp_HudTraining);
    PATCH_CALL(0x004555DE,CoOp_HudSharedText);
    DWORD ignored;VirtualProtect(reinterpret_cast<void*>(0x00401000),0x13A000,oldProtect,&ignored);
    DWORD trampolineProtect;VirtualProtect(tramp,64,PAGE_EXECUTE_READ,&trampolineProtect);
    FlushInstructionCache(GetCurrentProcess(),0,0);
    gHudInstalled=true;return true;
}
#endif
