#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef unsigned long u32;typedef long i32;typedef unsigned char u8;typedef unsigned short u16;
struct CVector {i32 vx,vy,vz;};struct Matrix {short m[3][3];};
struct CPlayer {
    CVector mPos,mVel,mAcc,mAngles,field_A8;Matrix mTransform;
    int mFlags,mCBodyFlags,mHealth,mMaxHealth,field_E1C,mCollision,mWebbing,field_5D8,field_5EC;bool field_5E9;
    u8 field_1AC;
    u8 padding[0x900];
};
typedef CPlayer CBody;struct CCamera {};
enum CoOpPlayerSlot {COOP_PLAYER_ONE,COOP_PLAYER_TWO};
struct CoOpPlayerHandle {unsigned slot,generation;CoOpPlayerHandle():slot(2),generation(0) {}};
static u8 mathScratch[0x100];
static CPlayer players[2];static CCamera cameras[2],*currentCamera=&cameras[0];
static int hudOffset;
static int checkpoint,paused,letterbox,context,originalDeaths,starts;static bool floorAvailable=true;
#include "coop/core/coop_session.h"
static CoOpSession gCoOpSession;
static void __fastcall SetArmor(CPlayer* p,void*,bool armored){p->field_5E9=armored;}
static struct Registry {
    bool joined;
    CoOpPlayerHandle Handle(CoOpPlayerSlot i) const {CoOpPlayerHandle h;h.slot=i;h.generation=1;return h;}
    const Registry& Router() const{return *this;}
    CPlayer* Get(CoOpPlayerSlot i){return i==0 || joined?&players[i]:0;}
    bool WithSecondaryContext(void (*fn)(void*),void* p){if(!joined)return false;context=1;currentCamera=&cameras[1];fn(p);context=0;currentCamera=&cameras[0];return true;}
} gCoOpPlayers;
static bool CoOp_FindSpawn(CPlayer* p,CVector& out){memset(mathScratch,0xA5,sizeof(mathScratch));out=p->mPos;out.vz+=64*4096;out.vy=-12*4096;return floorAvailable;}
static int __fastcall IsDead(CBody* p,void*){return (p->mCBodyFlags&0x40)!=0;}
static void __fastcall DeathMode(CPlayer*,void*,bool){++originalDeaths;}
static i32 __cdecl Floor(CVector*,i32,i32,CBody**){return floorAvailable?0:-1;}
static void __fastcall Orient(CPlayer* p,void*,short*){assert(p==&players[context]);}
static void __fastcall Stand(CPlayer* p,void*){assert(p==&players[context]);p->field_E1C=1;}
static void __fastcall FloorCamera(CPlayer* p,void*,int){assert(p==&players[context]);}
static void __fastcall CameraStart(CCamera* c,void*){assert(c==currentCamera);++starts;}
#include "life_runtime_test.h"
#include "coop/world/coop_fall_triggers.h"
struct SCommandPoint {u16* pCommands;unsigned NodeIndex,Checksum;SCommandPoint *pNext,*pNextSimilar;};
static u16* nodes[512];static int nodeCount;static SCommandPoint points[512],*pointList,*hashTable[256];
#include "fall_fixture.h"
#include "fall_trigger_runtime_test.h"
static void Reset(){memset(players,0,sizeof(players));gCoOpLife.Clear();
    gCoOpRevivePrimaryNextLevel=false;gCoOpSession.Select(true);gCoOpSession.enabled=true;gCoOpSession.pending=false;
    gCoOpLifeHandles[0]=gCoOpLifeHandles[1]=CoOpPlayerHandle();
    gCoOpLifeInstalled=true;gCoOpPlayers.joined=true;checkpoint=0;paused=letterbox=context=0;
    for(int i=0;i<2;++i){players[i].mHealth=players[i].mMaxHealth=100;players[i].mCollision=2;}
    memset(mathScratch,0x3C,sizeof(mathScratch));CoOp_LifeSync();
    assert(mathScratch[0]==0x3C && mathScratch[255]==0x3C);
}
int main(){
    LoadBombFallFixture();assert(!CoOp_FallChain(299,CoOp_ReadFallNode,0));
    LoadFallFixture();
    for(unsigned node=28;node<=32;++node) {
        assert(CoOp_FallChain(node,CoOp_ReadFallNode,0));
        for(unsigned owner=1;owner<=2;++owner) {
            Reset();assert(CoOp_InterceptFallTrigger(points[node].Checksum,owner));
            assert(players[owner-1].mHealth==75 && players[2-owner].mHealth==100 && !letterbox);
            assert(CoOp_InterceptFallTrigger(points[node].Checksum,owner));assert(players[owner-1].mHealth==75);
            assert(CoOp_InterceptFallTrigger(points[node].Checksum,3-owner));assert(players[2-owner].mHealth==75);
            CoOp_LifeFrame();assert(!gCoOpLife.slot[0].recover && !gCoOpLife.slot[1].recover);
        }
    }
    assert(!CoOp_FallChain(27,CoOp_ReadFallNode,0)); // ordinary level transition
    assert(!CoOp_FallChain(47,CoOp_ReadFallNode,0)); // ordinary question-mark cinematic
    assert(!CoOp_FallChain(257,CoOp_ReadFallNode,0)); // restart setup is not a fall
    Reset();gCoOpPlayers.joined=false;assert(!CoOp_InterceptFallTrigger(points[28].Checksum,1));
    puts("PASS installed rooftop asset: all five fall triggers, actual linked camera/actor/death graph, correct collider, both players same trigger, repeated collision, safe recovery, unrelated scripts and singleplayer");

    for(unsigned owner=0;owner<2;++owner) {
        Reset();CPlayer* p=&players[owner];p->field_1AC=1;
        letterbox=hudOffset=30;
        CoOp_LifeFall(p);
        assert(p->mHealth==75 && gCoOpLife.slot[owner].recover);
        assert(CoOp_LifeBefore(p)); // native crane route must keep ticking
        CoOp_LifeFrame();assert(gCoOpLife.slot[owner].recover);
        p->field_1AC=0;assert(!CoOp_LifeBefore(p));
        letterbox=0;CoOp_LifeFrame();assert(!gCoOpLife.slot[owner].recover);

        Reset();p=&players[owner];p->mHealth=25;p->field_1AC=1;
        letterbox=hudOffset=30;CoOp_LifeFall(p);
        assert(CoOp_LifeWaiting(owner) && !p->field_1AC &&
               !letterbox && !hudOffset && !CoOp_LifeBefore(p));
    }

    for(unsigned falling=0;falling<2;++falling){
        Reset();CPlayer* p=&players[falling];
        p->mWebbing=333;p->field_5D8=2;p->field_5E9=true;p->field_5EC=39;
        for(int n=1;n<=4;++n){
            p->mVel.vy=100;p->field_E1C=4;
            CoOp_PlayerDie(p,0);assert(p->mHealth==100-25*n);
            CoOp_PlayerDie(p,0);assert(p->mHealth==100-25*n); // one charge per fall
            assert(!CoOp_GameOver(&players[0],0));
            CoOp_LifeFrame();
            if(n<4) {
                assert(p->field_E1C==1 && !gCoOpLife.slot[falling].recover);
                assert(p->mWebbing==333 && p->field_5D8==2 && p->field_5E9 && p->field_5EC==39);
            }
        }
        assert(CoOp_LifeWaiting(falling) && !CoOp_LifeBefore(p));
        int before=p->mHealth;CoOp_LifeFrame();assert(p->mHealth==before);
        checkpoint=1;floorAvailable=false;CoOp_LifeFrame();
        assert(CoOp_LifeWaiting(falling) && gCoOpLife.slot[falling].revive);
        CoOp_DeathMode(p,0,false);assert(gCoOpLife.slot[falling].revive);
        floorAvailable=true;paused=1;CoOp_LifeFrame();assert(CoOp_LifeWaiting(falling));
        paused=0;letterbox=1;CoOp_LifeFrame();assert(CoOp_LifeWaiting(falling));
        letterbox=0;CoOp_LifeFrame();assert(mathScratch[0]==0x3C && mathScratch[255]==0x3C);
        assert(p->mHealth==100 && !CoOp_LifeWaiting(falling) && p->field_E1C==1);
        assert(p->mWebbing==4096 && p->field_5D8==7 && !p->field_5E9 && !p->field_5EC);
        CoOp_DeathMode(p,0,false);assert(CoOp_LifeWaiting(falling));
        CoOp_LifeSection("Second_",true,1u<<(1-falling));CoOp_LifeFrame();
        assert(p->mHealth==100 && !CoOp_LifeWaiting(falling));
        CoOp_DeathMode(p,0,false);CoOp_LifeSection("First_",true,1u<<(1-falling));CoOp_LifeFrame();
        assert(CoOp_LifeWaiting(falling)); // earlier sections are not new checkpoints
        CoOp_DeathMode(&players[1-falling],0,false);
        assert(CoOp_GameOver(&players[0],0));checkpoint=2;CoOp_LifeFrame();assert(gCoOpLife.BothDead());
    }
    for(unsigned scriptOwner=0;scriptOwner<2;++scriptOwner) {
        Reset();CPlayer* p=&players[scriptOwner];p->field_E1C=4;p->mVel.vy=10;
        letterbox=hudOffset=30;CoOp_ScriptDeath(p,0,true);
        assert(p->mHealth==75 && !letterbox && !hudOffset);
        CoOp_LifeFrame();assert(!gCoOpLife.slot[scriptOwner].recover);
    }
    Reset();players[0].mVel.vy=5;assert(CoOp_LegacyDeathEnd(&players[0]));assert(players[0].mHealth==75);
    Reset();CoOp_LifeDeath(&players[0]);assert(players[0].mHealth==0 && gCoOpLife.slot[0].waiting);
    CoOp_LifeRememberLevelChange();assert(gCoOpRevivePrimaryNextLevel);
    gCoOpLife.Clear();gCoOpPlayers.joined=false;gCoOpSession.pending=true;
    assert(!CoOp_GameOver(&players[0],0));
    CoOp_LifeResumePrimary(&players[0]);
    assert(players[0].mHealth==100 && !(players[0].mFlags&1) &&
        !(players[0].mCBodyFlags&0x40) && !gCoOpRevivePrimaryNextLevel);
    CoOpLife policy;policy.active=true;assert(policy.Fall(0,101,101)==75);
    gCoOpPlayers.joined=false;CoOp_DeathMode(&players[0],0,true);assert(originalDeaths==1);
    CoOp_PlayerDie(&players[0],0);assert(CoOp_GameOver(&players[0],0));
    puts("PASS co-op life: symmetric four-fall death, one charge, retained waiting panes/slots, checkpoint deferral/revival, forward-only sections, both-dead game-over, native singleplayer");
    return 0;
}
