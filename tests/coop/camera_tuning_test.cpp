#include <windows.h>
#include <assert.h>
#include <stdio.h>
typedef unsigned long u32;typedef unsigned short u16;
struct CPlayer {int xz,y,calls;};
static CPlayer p1={800,100,0},p2={800,100,0};static CPlayer* current=&p1;
enum {COOP_PLAYER_ONE,COOP_PLAYER_TWO};
static struct Registry {
    bool joined;
    bool InSecondaryContext(){return current==&p2;}
    CPlayer* Get(int slot){return slot==0?&p1:joined?&p2:0;}
    bool WithSecondaryContext(void (*fn)(void*),void* arg){
        if(!joined)return false;CPlayer* saved=current;current=&p2;fn(arg);current=saved;return true;
    }
} gCoOpPlayers;
static void __fastcall SetCamera(CPlayer* p,void*,u16 mode,u16 field,short value,u16 duration,u16 immediate){
    assert(current==p && duration==16 && immediate==1);++p->calls;
    if(mode==2 && field==3)p->xz=value;if(mode==2 && field==4)p->y=value;
}
#include "camera_tuning_runtime_test.h"
int main(){
    CoOp_SetCameraSetting(&p1,0,2,3,370,16,1);
    CoOp_SetCameraSetting(&p1,0,2,4,-700,16,1);
    CoOp_FlushCameraSettings();assert(p1.calls==2 && !p2.calls);
    gCoOpPlayers.joined=true;CoOp_FlushCameraSettings();
    assert(p2.xz==370 && p2.y==-700 && p2.calls==2 && current==&p1);
    CoOp_FlushCameraSettings();assert(p1.calls==2 && p2.calls==2);
    CoOp_SetCameraSetting(&p1,0,2,3,120,16,1);CoOp_FlushCameraSettings();assert(p2.xz==120);
    current=&p2;CoOp_SetCameraSetting(&p2,0,2,4,-400,16,1);current=&p1;
    CoOp_FlushCameraSettings();assert(p1.y==-400 && p2.y==-400);
    puts("PASS authored ceiling tuning: early/late joins, later changes, both directions, own player context, replay once");
    return 0;
}
