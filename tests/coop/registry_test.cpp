#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "router_test_types.h"
#include "ps2pad.h"
static unsigned char memoryImage[0x300000];
static u32 Address(u32 a) { assert(a>=0x400000 && a<0x700000);return (u32)(memoryImage+a-0x400000); }
#define CELL(T,a) (*reinterpret_cast<T*>(Address(a)))
static int allocated,released,rendered,playerTicks,cameraTicks,failAllocation;
static int ownedEffects;
static const char* expectedSuitResource="spReilly";
static int expectedSuit=8;
static u32 commands[]={0xABCD0001,0xBCDE0002,0xFFFFFFFF};
static CPlayer* liveP2;
static void* __cdecl Allocate(u32 size) {
    if(failAllocation && ++allocated==failAllocation) return 0;
    if(!failAllocation) ++allocated;
    return calloc(1,size);
}
static i32 __cdecl LoadSuit(const char* name,i32 env) {
    assert(!strcmp(name,expectedSuitResource) && env==0);
    assert(CELL(i32,0x5559DC)==expectedSuit && CELL(i32,0x6B78F8)==0);
    CELL(u8,0x6B2440+2*0x44+9)=CELL(u8,0x6B2440+2*0x44+10)=1;
    CELL(u32,0x6B2440+2*0x44+0x18)=2;
    return 2;
}
static CPlayer* FASTCALL Construct(CPlayer* p,int) {
    assert(CELL(u8,0x6A9040)==1); // constructor-only shared texture guard
    assert(CELL(CPlayer*,0x6A9038)==0 && CELL(u32,0x6A9034)==0);
    assert(CELL(void*,0x6B55A0)==0);
    CELL(CPlayer*,0x6A9038)=p;CELL(u32,0x6A9034)=1;
    CELL(u32,0x548860)=999;
    CELL(short,0x6A825C)=800;CELL(short,0x6A8CAC)=100;
    commands[0]&=0xFFFF;commands[1]&=0xFFFF;
    assert(CELL(i32,0x5559DC)==expectedSuit && CELL(i32,0x6B78F8)==0);
    p->mHealth=100;p->mRegion=2;p->mAnim=3;
    p->mWebbing=CELL(i32,0x6828A0)?CELL(i32,0x6828A0):4096;
    p->field_5D8=CELL(i32,0x6828A4);p->field_5EC=CELL(i32,0x6828A8);
    liveP2=p;
    return p;
}
static void* FASTCALL CameraDestroy(void* p,int,u32 flags) {
    assert(flags==1 && CELL(void*,0x56F3B8)==p);
    CELL(void*,0x56F3B8)=0;CELL(u32,0x56F3B4)=0;
    ++released;free(p);return 0;
}
static u32 cameraVtable[1]={(u32)&CameraDestroy};
static CCamera* FASTCALL CameraConstruct(CCamera* p,int,CBody* target) {
    assert(CELL(void*,0x56F3B8)==0 && target==liveP2);
    p->vtable=cameraVtable;p->mTripod=target;
    CELL(CCamera*,0x56F3B8)=p;CELL(u32,0x56F3B4)=1;return p;
}
static void FASTCALL Start(void*,int) {
    assert(CELL(short,0x6A825C)==120 && CELL(short,0x6A8CAC)==-400);
}
static void FASTCALL Orient(CPlayer* p,int,short* angles) {
    assert(p==liveP2 && angles[1]==p->mAngles.vy);
}
static void FASTCALL Stand(CPlayer* p,int) {assert(p==liveP2);}
static void FASTCALL EntryAnim(CPlayer* p,int,i32 anim,i32 frame,i32 target) {
    assert(p==liveP2 && CELL(CPlayer*,0x6A9038)==p);
    p->mAnim=(u16)anim;p->mFrame=(short)frame;p->mTargetFrame=(short)target;
    commands[0]&=0xffff; // native SFX flags stay private in the scoped bank
}
static void* FASTCALL EntrySwing(void*,int,const void*,i32,const void*,const void*) {
    assert(false);return 0; // this lifecycle test never creates a rope
}
static void __cdecl EntryFree(void* p){free(p);}
static void FASTCALL Load(void*,int) {CELL(u32,0x56F1B4)=1234;}
static void FASTCALL Update(void* p,int) {
    assert(CELL(i32,0x5559DC)==expectedSuit && CELL(i32,0x6B78F8)==0);
    assert(CELL(CPlayer*,0x6A9038)==liveP2 && CELL(u32,0x548860)==999);
    CELL(u32,0x610B20)=0xDEADBEEF; // transient geometry registers used by surface/camera math
    if(p==liveP2) {
        ++playerTicks;
        SControl* c=(SControl*)liveP2->field_E0C;
        assert(c!=(SControl*)Address(0x661100));
        assert(CELL(SControl,0x661100).Right.Pressed==c->Right.Pressed);
        if(c->Right.Pressed) liveP2->mPos.vx+=4096;
        // A player's action changes shared world state, retained after scope.
        CELL(u32,0x60DAAC)+=1;
        CELL(void*,0x6B55A0)=&ownedEffects;
        commands[0]=0x12340001;
    } else {assert(p==CELL(void*,0x56F3B8));++cameraTicks;}
}
static void* FASTCALL Destroy(void* p,int,u32 flags) {
    assert(p==liveP2 && flags==1);
    assert(CELL(i32,0x60CFA4)==1 && CELL(u8,0x6A9040)==0);
    assert(CELL(void*,0x6B55A0)==(playerTicks ? &ownedEffects : 0));
    CELL(void*,0x6B55A0)=0;
    CELL(void*,0x6A9038)=0;CELL(u32,0x6A9034)=0;
    CELL(u32,0x660F80)=0;++released;free(p);liveP2=0;return 0;
}
static void __cdecl Render(void* p) {assert(p==liveP2);++rendered;}
static void __cdecl Button(SButton* b,int held) {
    ++b->TriggeredTime;
    if(!b->Pressed && held) {b->Triggered=1;b->TriggeredTime=0;}
    b->Pressed=held!=0;
    if(held) {++b->PressedTime;b->ReleasedTime=0;}
    else {b->PressedTime=0;++b->ReleasedTime;}
}
static unsigned long fakeTime;
static bool noSpawnFloor;
static bool chaseFloor;
static unsigned long SpawnNow(){return fakeTime;}
static bool SpawnBlocked(){return false;}
static i32 __cdecl SpawnFloor(CVector* point,i32,i32,CBody**){
    if(chaseFloor)return point->vy<0 && point->vx>=17*4096 ? -128*4096:-1;
    return noSpawnFloor?-1:0;
}
static i32 __cdecl SpawnClear(CVector*,CVector*,CVector*,i32){return 1;}
static void __cdecl SpawnDirection(CVector* out,int distance,short* angles){
    out->vy=0;out->vx=angles[1]==1024?-distance*4096:0;
    out->vz=angles[1]==1024?0:-distance*4096;
}
#include "coop/core/coop_players.h"
static void PrivateTarget(void*) {CELL(i32,0x6A7488)=999;CELL(u8,0x6A7572)=1;}
static void CheckTarget(void*) {assert(CELL(i32,0x6A7488)==999 && CELL(u8,0x6A7572)==1);}
static void ReadHud(void* argument) {
    assert(CELL(CPlayer*,0x6A9038)==liveP2);
    assert(CELL(u32,0x548860)==999);
    *static_cast<int*>(argument)=liveP2->mHealth;
}
static void ArmorPickup(void*) {
    assert(CELL(CPlayer*,0x6A9038)==liveP2);
    assert(CELL(u8,0x6A9040)==0 && !liveP2->field_5E9);
    // Native SetArmor returns early if the constructor guard was left set.
    // Its successful path enables this body and sets this context's HUD art.
    CELL(u32,0x60F754)=0x1234;
    liveP2->field_5E9=true;liveP2->field_5EC=200;CELL(u8,0x6A9040)=1;
}
static void ArmorHud(void*) {
    assert(liveP2->field_5E9 && CELL(u8,0x6A9040)==1);
    assert(CELL(u32,0x60F754)==0x1234);
}
int main() {
    CELL(u32,0x6828D8)=1u<<7;
    CELL(i32,0x5559DC)=3;CELL(i32,0x6B78F8)=1;
    CPlayer p1;CCamera cam1;memset(&p1,0,sizeof(p1));memset(&cam1,0,sizeof(cam1));
    p1.mCollision=2;p1.field_A8.vy=-4096;p1.mMaxHealth=100;p1.mRegion=1;p1.mHealth=100;p1.mPos.vx=4096;p1.mAnim=7;
    CELL(CPlayer*,0x6A9038)=&p1;CELL(u32,0x6A9034)=1;
    CELL(CCamera*,0x56F3B8)=&cam1;CELL(u32,0x56F3B4)=1;
    CELL(short,0x6A825C)=120;CELL(short,0x6A8CAC)=-400;
    CELL(u32,0x548860)=444;CELL(u32,0x56F1B4)=777;
    CELL(void*,0x6B55A0)=&p1;CELL(u32,0x660F80)=88;
    CELL(u32,0x60CFA4)=3;CELL(u8,0x6A9040)=1;
    CELL(u8,0x6B2440+0x44+9)=CELL(u8,0x6B2440+0x44+10)=1;
    CELL(u32,0x6B2440+0x44+0x18)=1;
    CELL(u32*,0x6A82B8)=commands;
    CELL(u32*,0x6A82BC)=commands+1; // aliased command storage
    CoOpPlayerRegistry r;
    r.RequestSecondarySpawn();r.RenderSecondary();
    assert(allocated==2 && released==0 && rendered==1);
    CPlayer* p2=r.Get(COOP_PLAYER_TWO);
    assert(p2 && p2!=&p1 && p2->mHealth==100 && p2->mPos.vx==4096 && p2->mPos.vz==64*4096 && p2->mPos.vy==-12*4096);
    assert(p2->mAnim==7 && p1.mAnim==7 && p2->mRegion!=p1.mRegion);
    p2->mWebbing=1800;p2->field_5D8=4;p2->field_5EC=2;
    CoOpPlayerCarry carried;r.CaptureCarry(carried,1);
    assert(carried.valid && carried.webbing==1800 && carried.cartridges==4 && carried.armorHealth==2);
    r.SetCarry(&carried);
    assert(CELL(i32,0x5559DC)==3 && CELL(i32,0x6B78F8)==1);
    assert(CELL(CPlayer*,0x6A9038)==&p1 && CELL(CCamera*,0x56F3B8)==&cam1);
    assert(CELL(u32,0x548860)==444 && CELL(u32,0x56F1B4)==777);
    assert(commands[0]==0xABCD0001 && commands[1]==0xBCDE0002);
    assert(CELL(void*,0x6B55A0)==&p1);
    CoOpPlayerHandle old=r.Router().Handle(COOP_PLAYER_TWO);
    assert(r.Router().Resolve(r.RouteOwned(COOP_INPUT,old))->player==p2);
    r.RenderSecondary();assert(playerTicks==0 && cameraTicks==0); // rendering cannot simulate
    noSpawnFloor=true;
    r.RequestSecondarySpawn();r.RenderSecondary();fakeTime+=351;r.RenderSecondary();
    assert(r.Get(COOP_PLAYER_TWO)==p2 && r.Router().Resolve(old) && released==0);
    noSpawnFloor=false;
    int hudHealth=0;
    assert(r.WithSecondaryContext(ReadHud,&hudHealth) && hudHealth==100);
    CELL(i32,0x6A7488)=777;CELL(u8,0x6A7572)=0;
    assert(r.WithSecondaryContext(PrivateTarget,0));
    assert(CELL(i32,0x6A7488)==777 && CELL(u8,0x6A7572)==0);
    assert(r.WithSecondaryContext(CheckTarget,0));
    assert(playerTicks==0 && cameraTicks==0 && r.SecondaryTicks()==0);
    assert(CELL(CPlayer*,0x6A9038)==&p1 && CELL(CCamera*,0x56F3B8)==&cam1);
    assert(CELL(u32,0x548860)==444);
    CELL(u32,0x60F754)=0x5678; // independently armored P1's portrait
    assert(r.WithSecondaryContext(ArmorPickup,0));
    assert(r.WithSecondaryContext(ArmorHud,0));
    assert(CELL(u8,0x6A9040)==1 && CELL(u32,0x60F754)==0x5678);
    assert(!r.WithSecondaryContext(0,0));
    CoOpInput input;input.x=1;input.held=COOP_JUMP|COOP_PUNCH;
    CELL(u32,0x610B20)=0x12345678;
    r.TickSecondary(input);
    assert(playerTicks==1 && cameraTicks==0 && CELL(u32,0x610B20)==0x12345678);
    r.UpdateSecondaryCamera();
    assert(CELL(u32,0x610B20)==0x12345678);
    assert(playerTicks==1 && cameraTicks==1 && r.SecondaryTicks()==1);
    assert(p2->mPos.vx==2*4096 && p1.mPos.vx==4096);
    assert(CELL(u32,0x60DAAC)==1); // shared world effects survive
    assert(CELL(void*,0x6B55A0)==&p1 && commands[0]==0xABCD0001);
    SControl* control=(SControl*)p2->field_E0C;
    assert(control->Crouch.Triggered && control->SmartBomb.Triggered);
    r.TickSecondary(input);r.UpdateSecondaryCamera();assert(!control->Crouch.Triggered && control->Crouch.PressedTime==2);
    int beforePlayerTicks=playerTicks,beforeCameraTicks=cameraTicks;
    r.UpdateSecondaryCamera(); // cinematic: camera progresses without another player tick
    assert(playerTicks==beforePlayerTicks && cameraTicks==beforeCameraTicks+1);
    unsigned char view[0x94];assert(r.CopySecondaryCamera(view));
    assert(*(u32*)(view+4)==1234 && CELL(u32,0x56F1B4)==777);
    r.RequestSecondaryRemoval();assert(!r.Get(COOP_PLAYER_TWO) && released==0);
    r.RenderSecondary();assert(released==2 && !r.Router().Resolve(old));
    assert(CELL(void*,0x6B55A0)==&p1 && CELL(u32,0x660F80)==88);
    assert(CELL(u32,0x60CFA4)==3 && CELL(u8,0x6A9040)==1);
    playerTicks=0;r.RequestSecondarySpawn();r.RenderSecondary();fakeTime+=351;r.RenderSecondary();
    assert(r.Get(COOP_PLAYER_TWO) && !r.Router().Resolve(old));
    r.Get(COOP_PLAYER_TWO)->mHealth=0;r.TickSecondary(input);
    assert(playerTicks==0 && r.Get(COOP_PLAYER_TWO));
    r.RequestSecondaryRemoval();r.RenderSecondary();assert(released==2 && r.Get(COOP_PLAYER_TWO));
    r.Get(COOP_PLAYER_TWO)->mHealth=100;
    r.RequestSecondaryRemoval();r.RenderSecondary();assert(released==4);
    // Failed camera allocation rolls back the already constructed player.
    failAllocation=allocated+2;r.RequestSecondarySpawn();r.RenderSecondary();fakeTime+=351;r.RenderSecondary();
    assert(!r.Get(COOP_PLAYER_TWO) && released==5 && CELL(CPlayer*,0x6A9038)==&p1);
    failAllocation=0;r.Clear();r.Clear();assert(released==5);
    assert(r.SyncPrimaryFromRetail() && !r.ConsumePrimaryReplacement());
    // The selected costume drives both the resource and scoped CurrentSuit.
    // Loading another slot must reject a suit that slot has not unlocked.
    CoOpRetailPlayer costume;
    gCoOpSelectedSuit=2;CELL(u32,0x6828D8)=1u<<1;
    expectedSuit=2;expectedSuitResource="sp2099";
    assert(costume.Create(&p1) && costume.Get());
    assert(CELL(i32,0x5559DC)==3 && CELL(i32,0x6B78F8)==1);
    costume.Destroy();
    gCoOpSelectedSuit=10;CELL(u32,0x6828D8)=0;
    expectedSuit=8;expectedSuitResource="spReilly";
    assert(costume.Create(&p1) && costume.Get());
    assert(CELL(i32,0x5559DC)==3 && CELL(i32,0x6B78F8)==1);
    costume.Destroy();
    gCoOpSelectedSuit=8;CELL(u32,0x6828D8)=1u<<7;
    expectedSuit=8;expectedSuitResource="spReilly";
    memcpy(reinterpret_cast<void*>(Address(0x6B2440+9*0x44)),"L3A2_G",8);
    CELL(u8,0x6B2440+9*0x44+10)=1;
    chaseFloor=true;CoOpRetailPlayer chase;
    assert(chase.Create(&p1) && chase.SpawnStrategy()==2);
    assert(chase.Get()->mPos.vx==p1.mPos.vx+24*4096 &&
        chase.Get()->mPos.vy==-140*4096);
    chase.Destroy();chaseFloor=false;
    CELL(CPlayer*,0x6A9038)=0;
    assert(!r.SyncPrimaryFromRetail());
    CPlayer nextLevel;memset(&nextLevel,0,sizeof(nextLevel));
    CELL(CPlayer*,0x6A9038)=&nextLevel;
    assert(r.SyncPrimaryFromRetail() && r.ConsumePrimaryReplacement());
    assert(!r.ConsumePrimaryReplacement());
    // A scripted/airborne level opening creates a hidden native player now,
    // follows P1 without simulation, then releases at supported collision.
    nextLevel.mHealth=100;nextLevel.mRegion=1;nextLevel.field_1AC=1;
    nextLevel.mAnim=227;nextLevel.mFrame=9;nextLevel.mAnimSpeed=0x10000;
    noSpawnFloor=true;r.RequestSecondarySpawn();
    int drawsBefore=rendered,ticksBefore=playerTicks;
    r.RenderSecondary();assert(r.Get(COOP_PLAYER_TWO) && r.SecondaryShadowed());
    assert(r.Get(COOP_PLAYER_TWO)->mWebbing==1800 &&
        r.Get(COOP_PLAYER_TWO)->field_5D8==4 && r.Get(COOP_PLAYER_TWO)->field_5EC==2);
    assert(rendered==drawsBefore && playerTicks==ticksBefore);
    assert(r.Get(COOP_PLAYER_TWO)->mAnim==227 && r.Get(COOP_PLAYER_TWO)->mFrame==9);
    nextLevel.mPos.vx=10*4096;r.TickSecondary(input);
    assert(r.Get(COOP_PLAYER_TWO)->mPos.vx==nextLevel.mPos.vx && playerTicks==ticksBefore);
    nextLevel.field_1AC=0;
    // Native control returns on a ceiling with no world-Y floor underneath.
    *((u32*)((u8*)&nextLevel+0xE1C))=1;
    *((short*)((u8*)&nextLevel+0xAA))=4096;
    *((u8*)&nextLevel+0xAD4)=1;
    nextLevel.mVel.vx=321;nextLevel.mAcc.vy=-12;
    nextLevel.mFrame=17;nextLevel.mFrameFrac=123;
    r.RenderSecondary();
    assert(!r.SecondaryShadowed() && rendered==drawsBefore+1);
    assert(r.Get(COOP_PLAYER_TWO)->mPos.vx==nextLevel.mPos.vx);
    assert(r.SpawnStrategy()==4 && r.Get(COOP_PLAYER_TWO)->mVel.vx==321 &&
        r.Get(COOP_PLAYER_TWO)->mAcc.vy==-12 && r.Get(COOP_PLAYER_TWO)->mFrame==17 &&
        r.Get(COOP_PLAYER_TWO)->mFrameFrac==123 &&
        *((short*)((u8*)r.Get(COOP_PLAYER_TWO)+0xAA))==4096);
    r.TickSecondary(input);assert(playerTicks==ticksBefore+1);
    noSpawnFloor=false;
    r.Clear();
    puts("PASS: production CPlayer adapter allocation/lifecycle, private camera/input, original-update dispatch, restoration, shared world, death drop-out, join failure rollback");
    return 0;
}
