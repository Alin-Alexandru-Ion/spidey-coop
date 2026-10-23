#include <windows.h>
#include <assert.h>
#include <stdio.h>
typedef unsigned long u32;typedef long i32;typedef unsigned short u16;
enum ECameraMode {CAMERAMODE_DEMO=3,CAMERAMODE_START=4,CAMERAMODE_FAR=5,CAMERAMODE_USER=16};
struct CPlayer {int position;u16 locked;};
struct CCamera {ECameraMode mode;CPlayer* tripod;int position,angle;};
static CPlayer p1={10,0},p2={90,0};
static CCamera c1={CAMERAMODE_DEMO,&p1,10,20},c2={CAMERAMODE_FAR,&p2,90,40};
static CPlayer* currentPlayer=&p1;static CCamera* currentCamera=&c1;
static struct {bool enabled;} gCoOpSession={true};
static unsigned modeCalls[2],lockCalls[2];
static i32 __fastcall NativeMode(CCamera* c,void*,ECameraMode mode) {
    assert(currentCamera==c);int old=c->mode;c->mode=mode;++modeCalls[c==&c2];return old;
}
static void __fastcall NativeLock(CPlayer* p,void*,u16 locked) {
    assert(currentPlayer==p);p->locked=locked;++lockCalls[p==&p2];
}
static struct Registry {
    bool joined;
    bool InSecondaryContext(){return currentPlayer==&p2;}
    bool WithSecondaryContext(void(*fn)(void*),void* arg) {
        if(!joined)return false;
        CPlayer* p=currentPlayer;CCamera* c=currentCamera;
        currentPlayer=&p2;currentCamera=&c2;fn(arg);currentPlayer=p;currentCamera=c;return true;
    }
} gCoOpPlayers={true};
#include "camera_policy_runtime_test.h"
static void Tick(CCamera& c) {if(c.mode==CAMERAMODE_DEMO)c.position=c.tripod->position;}
int main() {
    // The actual sewer release command starts 160,3,187,0. Before the fix,
    // only P1 releases and P2 remains at the cinematic's fixed position.
    NativeMode(&c1,0,CAMERAMODE_DEMO);p2.position=100;Tick(c2);assert(c2.position==90);
    assert(CoOp_ScriptCameraMode(&c1,0,CAMERAMODE_DEMO)==CAMERAMODE_DEMO);
    CoOp_ScriptCameraLock(&p1,0,0);CoOp_FlushCameraPolicy();
    assert(c2.mode==CAMERAMODE_DEMO && !p2.locked && currentPlayer==&p1 && currentCamera==&c1);
    Tick(c2);assert(c2.position==100 && c2.tripod==&p2 && c2.angle==40 && c1.angle==20);
    unsigned calls=modeCalls[0]+modeCalls[1];CoOp_FlushCameraPolicy();assert(calls==modeCalls[0]+modeCalls[1]);
    // Fixed camera scenes remain fixed until an authored mode command runs;
    // merely ending letterbox/focusing a different player never resets them.
    c2.mode=CAMERAMODE_FAR;CoOp_FlushCameraPolicy();assert(c2.mode==CAMERAMODE_FAR);
    currentPlayer=&p2;currentCamera=&c2;
    CoOp_ScriptCameraMode(&c2,0,CAMERAMODE_DEMO);CoOp_ScriptCameraLock(&p2,0,0);
    currentPlayer=&p1;currentCamera=&c1;c1.mode=CAMERAMODE_FAR;p1.locked=1;
    CoOp_FlushCameraPolicy();assert(c1.mode==CAMERAMODE_DEMO && !p1.locked);
    // Keep only the latest authored rule before the frame boundary.
    CoOp_ScriptCameraMode(&c1,0,CAMERAMODE_FAR);
    CoOp_ScriptCameraMode(&c1,0,CAMERAMODE_USER);CoOp_ScriptCameraLock(&p1,0,1);
    CoOp_FlushCameraPolicy();assert(c2.mode==CAMERAMODE_USER && p2.locked==1);
    // Intro policies can arrive before P2 is constructed.
    gCoOpPlayers.joined=false;CoOp_ScriptCameraMode(&c1,0,CAMERAMODE_DEMO);
    CoOp_FlushCameraPolicy();assert(gCoOpCameraPolicy.modePending==2 && c2.mode==CAMERAMODE_USER);
    gCoOpPlayers.joined=true;CoOp_FlushCameraPolicy();assert(c2.mode==CAMERAMODE_DEMO);
    // New level clears pending rules; intact singleplayer only invokes retail.
    gCoOpCameraPolicy=CoOpCameraPolicy();gCoOpSession.enabled=false;
    c2.mode=CAMERAMODE_FAR;CoOp_ScriptCameraMode(&c1,0,CAMERAMODE_USER);
    CoOp_ScriptCameraLock(&p1,0,0);CoOp_FlushCameraPolicy();assert(c2.mode==CAMERAMODE_FAR && p2.locked==1);
    puts("PASS authored camera policy: native sewer release, both owners, independent tripods/poses, intentional fixed scenes, latest setting, late join, singleplayer");return 0;
}
