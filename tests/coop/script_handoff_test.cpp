#include "router_test_types.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static i32 clockNow=100;
static u32 Address(u32 a){assert(a==0x006B4CA8);return (u32)&clockNow;}
static void* __cdecl Allocate(u32 size){return calloc(1,size);}
static void __cdecl EntryFree(void* p){free(p);}
static void FASTCALL EntryAnim(CPlayer* p,int,i32 anim,i32 frame,i32 target){
    p->mAnim=(u16)anim;p->mFrame=(short)frame;p->mTargetFrame=(short)target;
}
static void* FASTCALL EntrySwing(void*,int,const void*,i32,const void*,const void*){assert(false);return 0;}
static void FASTCALL Start(CCamera*,int){}
static i32 __cdecl SpawnFloor(CVector*,i32,i32,CBody**){return -1;}
static i32 __cdecl SpawnClear(CVector*,CVector*,CVector*,i32){return 1;}
static void __cdecl SpawnDirection(CVector*,int,short*){assert(false);}
static bool SpawnBlocked(){return false;}
#include "coop/player/coop_spawn.h"
#include "coop/player/coop_entry_state.h"
#include "coop/script/coop_script_motion.h"
static CPlayer players[2];static CCamera cameras[2];
static CCamera* primaryCamera=&cameras[0];
static int letterbox,hudOffset,handoffs;
static bool waiting[2];
static bool CoOp_LifeWaiting(unsigned slot){return waiting[slot];}
static const void* scriptBank=(void*)0x1234;
static u8 mathScratch[0x100];
static CoOpScriptMotionState gCoOpScriptMotion;
static CoOpPlayerHandle gCoOpCinematicOwner;
static void CoOp_ReleaseDeadCinematic(){letterbox=hudOffset=0;gCoOpCinematicOwner=CoOpPlayerHandle();}
class TestPlayers {
    CoOpPlayerRouter roster;
public:
    void Reset(){roster.Clear();for(unsigned i=0;i<2;++i)roster.Publish((CoOpPlayerSlot)i,&players[i]);}
    const CoOpPlayerRouter& Router()const{return roster;}
    CPlayer* Get(CoOpPlayerSlot slot)const{return roster.Resolve(roster.Handle(slot))->player;}
    CCamera* SecondaryCamera(){return &cameras[1];}
    void ShadowSecondary(CPlayer* actor){CoOp_ShadowEntry(&players[1],actor);}
    bool HandoffSecondary(CPlayer* actor,const CVector& position,CCamera* actorCamera){
        ++handoffs;bool ok=CoOp_AdoptEntryMotion(&players[1],actor,position);
        if(ok)CoOp_AdoptEntryCamera(&cameras[1],actorCamera);return ok;
    }
} gCoOpPlayers;
#include "script_handoff_runtime_test.h"
static void Reset(unsigned owner){
    memset(players,0,sizeof(players));memset(cameras,0,sizeof(cameras));
    waiting[0]=waiting[1]=false;letterbox=hudOffset=handoffs=0;
    players[0].mHealth=80;players[1].mHealth=60;
    players[0].mWebbing=2500;players[1].mWebbing=1000;
    players[owner].mAnim=227;players[owner].mFrame=11;
    players[owner].mVel.vx=44;players[owner].field_1AC=1;
    ((u8*)&players[owner])[0xAD4]=1;players[owner].field_A8.vy=4096;
    *(u32*)((u8*)&players[owner]+0xE1C)=1;
    *(u32*)((u8*)&cameras[owner]+0x2A0)=16;
    gCoOpPlayers.Reset();gCoOpScriptMotion.Clear();
    assert(gCoOpScriptMotion.Begin(gCoOpPlayers.Router().Handle((CoOpPlayerSlot)owner),
        scriptBank,players[owner].mPos,0));
}
int main(){
    for(unsigned owner=0;owner<2;++owner){
        Reset(owner);CPlayer& actor=players[owner];CPlayer& partner=players[1-owner];
        actor.mPos.vx=10*4096;letterbox=30;
        CoOp_ScriptMotionFrame();
        assert(partner.mPos.vx==actor.mPos.vx && partner.mAnim==227 && partner.mFrame==11);
        assert(gCoOpScriptMotion.active && !gCoOpScriptMotion.waitingForPartner && !handoffs);
        actor.field_1AC=0;actor.mPos.vx=200*4096;
        CoOp_ScriptMotionFrame();
        assert(gCoOpScriptMotion.waitingForPartner && !handoffs);
        CoOp_ScriptMotionFrame(true);assert(gCoOpScriptMotion.active); // letterbox still owns it
        // Later native camera/pending commands supply the FINAL authored exit.
        actor.mPos.vx=400*4096;actor.mFrame=18;letterbox=0;
        unsigned long ticks=gCoOpScriptMotion.ticks;
        CoOp_ScriptMotionFrame(true);
        assert(!gCoOpScriptMotion.active && partner.mPos.vx==400*4096 &&
            partner.mVel.vx==44 && partner.mFrame==18 && partner.field_A8.vy==4096);
        assert(players[0].mHealth==80 && players[1].mHealth==60 &&
            players[0].mWebbing==2500 && players[1].mWebbing==1000);
        assert(*(u32*)((u8*)&cameras[1-owner]+0x2A0)==16);
        assert(handoffs==(owner==0?1:0));
        CoOp_ScriptMotionFrame(true);assert(handoffs==(owner==0?1:0));
        Reset(owner);players[1-owner].mHealth=0;waiting[1-owner]=true;
        actor.field_1AC=0;CoOp_ScriptMotionFrame();CoOp_ScriptMotionFrame(true);
        assert(!gCoOpScriptMotion.active && !handoffs && players[1-owner].mHealth==0);
        Reset(owner);actor.mHealth=0;letterbox=30;
        CoOp_ScriptMotionFrame();assert(!gCoOpScriptMotion.active && !letterbox);
        Reset(owner);scriptBank=(void*)0x5678;
        CoOp_ScriptMotionFrame();assert(!gCoOpScriptMotion.active);scriptBank=(void*)0x1234;
    }
    puts("PASS production scripted handoff: both owners, final post-camera exit, ceiling/no-floor release, animation, separate inventory and waiting/dead/bank-change cleanup");
    return 0;
}
