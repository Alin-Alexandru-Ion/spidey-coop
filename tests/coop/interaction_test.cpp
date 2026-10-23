#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "coop/interaction/coop_interactions.h"
#include "coop/world/coop_sections.h"
#include "coop/script/coop_script_motion.h"
struct CBaddy {CVector mPos;i32 field_208;unsigned mType;int mHealth;u32 field_218,field_194;struct {unsigned bothFlags;} field_31C;};
struct CThug:CBaddy {};struct CCop:CBaddy {};struct CLizMan:CBaddy {};
struct CRhino:CBaddy {};
struct CVenom:CBaddy {};
struct CChopper:CBaddy {};
struct CMachineGunBullet {CVector mEnd;};
typedef void (*UserFunction)(const u32*,u32*);
struct reloc_mod {UserFunction field_C[8];};
static struct {bool enabled;} gCoOpSession={true};
struct CSniperTarget:CBody {};struct CSearchlight:CBody {};struct CChopperMissile:CBody {};
struct SHandle {void* pWhatever;u32 Id;};
struct CScorpion:CBaddy {i32 field_BF8;SHandle hCurrentTarget;};
struct CPowerUp {unsigned mType;bool consumed;};
struct PendingListEntry {unsigned short NodeIndex;unsigned short pad;u16* pCommands;};
static PendingListEntry queue[16];
static unsigned CoOp_PendingCount() {unsigned n=0;while(n<16 && queue[n].pCommands)++n;return n;}
static CoOpDeferredOwners gCoOpDeferredOwners;
static CoOpSectionOwners gCoOpNodeOwners;
static unsigned gCoOpTriggerOwner,gCoOpCommandOwners,gCoOpStaleScripts;
static unsigned gCoOpSharedLevelChanges;
static CoOpPlayerHandle gCoOpCinematicOwner;
static bool waitingPlayer[2];
static bool CoOp_LifeWaiting(unsigned slot) {return waitingPlayer[slot];}
static CoOpScriptMotionState gCoOpScriptMotion;
static i32 letterbox,hudOffset;
static bool interactiveScene;
static bool deferRaceFail;
static bool gCoOpRaceSectionsActive;
static unsigned gCoOpRaceEntryScripts[2],raceVisibilityTouches;
static void __cdecl CoOp_SectionVisibility(const char* prefix,i32 first,i32 last,bool visible) {
    assert(!strcmp(prefix,"Inside") && first==1 && last==1 && visible);
    ++raceVisibilityTouches;
}
static void* scriptBank;
struct SCommandPoint {u16* pCommands;unsigned NodeIndex;SCommandPoint* pNext;};
static u8 craneResources[40*0x44];
static u16** craneNodes;
static int craneNodeCount;
static SCommandPoint* cranePoints;
static u8 nativeSkipFlag;
#include "crane_fixture.h"
static CPlayer p1,p2;static CPlayer* mech=&p1;
static struct Registry {
    CoOpPlayerRouter router;bool secondary;unsigned contextCalls;
    const CoOpPlayerRouter& Router() const {return router;}
    CPlayer* Get(CoOpPlayerSlot s) {const CoOpPlayerRouter::Participant* p=router.Resolve(router.Handle(s));return p?p->player:0;}
    bool InSecondaryContext() const {return secondary;}
    bool WithSecondaryContext(void(*callback)(void*),void* a) {
        if(secondary || !Get(COOP_PLAYER_TWO))return false;
        CPlayer* old=mech;mech=&p2;secondary=true;++contextCalls;
        callback(a);secondary=false;mech=old;return true;
    }
} gCoOpPlayers;
static unsigned enemyCalls,pickupCalls,scriptCalls;static CPlayer* observed;
static bool enqueueActorPulse;static u16 chaseCameraScript[]={186,0xFFFF};
static void QueueActorPulse() {
    if(enqueueActorPulse) {unsigned n=CoOp_PendingCount();queue[n].NodeIndex=715;queue[n].pCommands=chaseCameraScript;}
}
static bool blockP2;static int scriptLetterbox=-1;static u16 firstScript[1],childScript[1];
static i32 __fastcall IsDead(const CPlayer* p,void*) {return p->mHealth<=0;}
static i32 __cdecl LineOfSight(const CVector*,const CVector* to,i32,i32) {return !(blockP2 && to==&p2.mPos);}
static void __fastcall NativeEnemy(CBaddy*,void*) {++enemyCalls;observed=mech;}
static unsigned lizCalls;static bool lizDamage;
static void __fastcall NativeLizMan(CBaddy* enemy,void*) {
    ++lizCalls;observed=mech;if(gCoOpPlayers.Get(COOP_PLAYER_TWO))assert(!enemy->field_208);
    if(lizDamage)mech->mHealth-=8;
}
static void __fastcall NativeChopper(CChopper*,void*) {++enemyCalls;observed=mech;QueueActorPulse();}
static void __fastcall NativeChaseBody(CBody*,void*) {++enemyCalls;observed=mech;QueueActorPulse();}
static unsigned bulletCalls;
static void __fastcall NativeBullet(CMachineGunBullet*,void*) {++bulletCalls;observed=mech;}
static void __cdecl NativeProgress(const u32*,u32*) {observed=mech;}
static void __cdecl NativeRelocInit(reloc_mod* module) {module->field_C[1]=NativeProgress;}
static unsigned raceScopeCalls;
static void CoOp_HudRaceProgress(u32 original,const u32* input,u32* output) {
    ++raceScopeCalls;reinterpret_cast<UserFunction>(original)(input,output);
}
static CBaddy* __cdecl ResolveEnemy(const SHandle* h){return static_cast<CBaddy*>(h->pWhatever);}
static void __fastcall ReleasePlayer(CPlayer*,void*,int,int,int){}
static SHandle __cdecl MakeHandle(void* p) {SHandle h={p,42};return h;}
static void __fastcall NativeScorpion(CScorpion* boss,void*) {
    ++enemyCalls;observed=mech;
    if(boss->field_BF8==2) assert(boss->hCurrentTarget.pWhatever==mech);
}
static bool __fastcall NativePickup(CPowerUp* p,void*,CPlayer* c) {
    ++pickupCalls;observed=c;
    if(p->mType==11 && !p->consumed) {
        p->consumed=true;unsigned n=CoOp_PendingCount();queue[n].NodeIndex=42;queue[n].pCommands=firstScript;
    }
    return true;
}
struct FloorItem {unsigned short mFlags;FloorItem* mNextItem;};
static FloorItem trailingFloor={0,0};
static CoOpSections<FloorItem> floorSections;
static bool hideFloor;
static unsigned observedSectionOwner;
static unsigned observedTriggerOwner;
static u16* observedScript;static i32 observedScriptNode;
static void __cdecl NativeScript(u16* script,i32 node,i32) {
    ++scriptCalls;observed=mech;observedSectionOwner=gCoOpCommandOwners;
    observedScript=script;observedScriptNode=node;observedTriggerOwner=gCoOpTriggerOwner;
    if(hideFloor)floorSections.Command(&trailingFloor,gCoOpCommandOwners,false);
    if(scriptLetterbox>=0) letterbox=scriptLetterbox;
    if(script==firstScript) {unsigned n=CoOp_PendingCount();queue[n].NodeIndex=43;queue[n].pCommands=childScript;}
}
#include "script_dispatch_test.h"
#include "coop/script/coop_cinematic_tick.h"
#include "coop/camera/coop_aim.h"
#include "aim_runtime_test.h"
#include "interaction_runtime_test.h"
int main() {
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    memset(&p1,0,sizeof(p1));memset(&p2,0,sizeof(p2));p1.mHealth=p2.mHealth=100;
    p1.mPos.vx=100*4096;p2.mPos.vx=10*4096;
    gCoOpPlayers.router.Publish(COOP_PLAYER_ONE,&p1);gCoOpPlayers.router.Publish(COOP_PLAYER_TWO,&p2);
    CThug enemy;memset(&enemy,0,sizeof(enemy));enemy.field_208=123;
    CoOp_ThugAI(&enemy,0);assert(enemyCalls==1 && observed==&p2 && mech==&p1 && !enemy.field_208);
    blockP2=true;CoOp_ThugAI(&enemy,0);assert(enemyCalls==2 && observed==&p1);
    blockP2=false;p1.mPos.vx=4096;CoOp_ThugAI(&enemy,0);assert(enemyCalls==3 && observed==&p1);
    p1.mPos.vx=100*4096;CCop cop;memset(&cop,0,sizeof(cop));CoOp_CopAI(&cop,0);
    assert(enemyCalls==4 && observed==&p2 && mech==&p1);
    CLizMan liz;memset(&liz,0,sizeof(liz));liz.mType=317;
    liz.field_208=999;lizDamage=true;
    CoOp_LizManAI(&liz,0);assert(lizCalls==1 && observed==&p2 && p2.mHealth==92 && p1.mHealth==100 && mech==&p1);
    blockP2=true;CoOp_LizManAI(&liz,0);assert(lizCalls==2 && observed==&p1 && p1.mHealth==92);
    blockP2=false;p1.mHealth=0;CoOp_LizManAI(&liz,0);assert(lizCalls==3 && observed==&p2 && p2.mHealth==84);
    p1.mHealth=100;p2.mHealth=0;CoOp_LizManAI(&liz,0);assert(lizCalls==4 && observed==&p1);
    assert(gCoOpLizManTargets[0]==2 && gCoOpLizManTargets[1]==2);
    p1.mHealth=p2.mHealth=100;lizDamage=false;
    CRhino rhino;memset(&rhino,0,sizeof(rhino));rhino.mPos.vx=10*4096;
    rhino.mType=307;
    CoOp_RhinoAI(&rhino,0);assert(observed==&p2 && mech==&p1);
    p2.mHealth=0;CoOp_RhinoAI(&rhino,0);assert(observed==&p1 && mech==&p1);
    assert(gCoOpRhinoTargets[0]==1 && gCoOpRhinoTargets[1]==1);
    p2.mHealth=100;
    CVenom venom;memset(&venom,0,sizeof(venom));venom.mPos.vx=10*4096;
    CoOp_VenomAI(&venom,0);assert(observed==&p2 && mech==&p1);
    CoOp_VenomProgress(0,0);assert(observed==&p2 && mech==&p1);
    reloc_mod module;memset(&module,0,sizeof(module));CoOp_VenomRelocInit(&module);
    assert(module.field_C[1]==CoOp_VenomProgress);
    module.field_C[1](0,0);assert(observed==&p2 && mech==&p1);
    CoOp_L2RaceRelocInit(&module);assert(module.field_C[1]==CoOp_L2RaceProgress);
    module.field_C[1](0,0);assert(observed==&p1);
    CoOp_L5RaceRelocInit(&module);assert(module.field_C[1]==CoOp_L5RaceProgress);
    module.field_C[1](0,0);assert(observed==&p1 && raceScopeCalls==4);
    CoOp_SuperOckRaceRelocInit(&module);assert(module.field_C[1]==CoOp_SuperOckRaceProgress);
    module.field_C[1](0,0);assert(observed==&p1 && raceScopeCalls==5);
    p2.mHealth=0;CoOp_VenomAI(&venom,0);assert(observed==&p1 && mech==&p1);
    CoOp_VenomProgress(0,0);assert(observed==&p1 && mech==&p1);
    assert(gCoOpVenomTargets[0]==1 && gCoOpVenomTargets[1]==1);
    p2.mHealth=100;
    // A native grab phase must keep its original victim even when the other
    // participant becomes closer, then permit retargeting once the phase ends.
    venom.field_31C.bothFlags=0x100;
    CoOp_VenomAI(&venom,0);assert(observed==&p2 && gCoOpVenomGrabOwner.slot==COOP_PLAYER_TWO);
    p1.mPos.vx=venom.mPos.vx;
    CoOp_VenomAI(&venom,0);assert(observed==&p2 && gCoOpVenomGrabOwner.slot==COOP_PLAYER_TWO);
    venom.field_31C.bothFlags=0;
    CoOp_VenomAI(&venom,0);assert(observed==&p1 && gCoOpVenomGrabOwner.slot==COOP_PLAYER_COUNT);
    venom.field_31C.bothFlags=0x100;
    CoOp_VenomAI(&venom,0);assert(observed==&p1 && gCoOpVenomGrabOwner.slot==COOP_PLAYER_ONE);
    p1.mPos.vx=100*4096;
    CoOp_VenomAI(&venom,0);assert(observed==&p1 && gCoOpVenomGrabOwner.slot==COOP_PLAYER_ONE);
    venom.field_31C.bothFlags=0;CoOp_VenomAI(&venom,0);
    p2.mHealth=0;CoOp_ThugAI(&enemy,0);assert(observed==&p1);p2.mHealth=100;
    CChopper chopper;memset(&chopper,0,sizeof(chopper));chopper.mPos.vx=10*4096;
    p1.mHealth=0;CoOp_ChopperAI(&chopper,0);
    assert(observed==&p2 && gCoOpChopperTargets[1]==1 && mech==&p1);
    p1.mHealth=100;CoOp_ChopperAI(&chopper,0);
    assert(observed==&p2 && gCoOpChopperTargets[1]==2);
    p2.mHealth=0;CoOp_ChopperAI(&chopper,0);
    assert(observed==&p1 && gCoOpChopperTargets[0]==1);p2.mHealth=100;
    CSniperTarget sniper;memset(&sniper,0,sizeof(sniper));sniper.mPos.vx=10*4096;
    CSearchlight light;memset(&light,0,sizeof(light));light.mPos.vx=10*4096;
    CChopperMissile missile;memset(&missile,0,sizeof(missile));missile.mPos.vx=10*4096;
    p1.mHealth=0;CoOp_SniperAI(&sniper,0);CoOp_SearchlightAI(&light,0);
    CoOp_MissileAI(&missile,0);
    assert(observed==&p2 && gCoOpChaseTargets[1]==3 && mech==&p1);
    p2.mHealth=0;p1.mHealth=100;CoOp_MissileAI(&missile,0);
    assert(observed==&p1 && gCoOpChaseTargets[0]==1);
    p2.mHealth=100;
    CMachineGunBullet bullet;memset(&bullet,0,sizeof(bullet));
    bullet.mEnd.vx=p2.mPos.vx;CoOp_BulletMove(&bullet,0);
    assert(bulletCalls==1 && observed==&p2 && mech==&p1 && gCoOpBulletTargets[1]==1);
    bullet.mEnd.vx=p1.mPos.vx;CoOp_BulletMove(&bullet,0);
    assert(bulletCalls==2 && observed==&p1 && gCoOpBulletTargets[0]==1);
    p1.mHealth=0;bullet.mEnd.vx=p1.mPos.vx;CoOp_BulletMove(&bullet,0);
    assert(bulletCalls==3 && observed==&p2 && gCoOpBulletTargets[1]==2);
    p1.mHealth=100;
    enqueueActorPulse=true;memset(queue,0,sizeof(queue));p1.mHealth=0;
    CoOp_ChopperAI(&chopper,0);
    assert(gCoOpDeferredOwners.Get(chaseCameraScript,715).slot==COOP_PLAYER_TWO &&
        gCoOpNodeOwners.Get(715)==2 && gCoOpChaseActorPulses[1]==1);
    CoOp_SectionExecute(chaseCameraScript,715,0);assert(observed==&p2);
    memset(queue,0,sizeof(queue));gCoOpDeferredOwners.Clear();gCoOpNodeOwners.Clear();
    CoOp_SearchlightAI(&light,0);
    assert(gCoOpDeferredOwners.Get(chaseCameraScript,715).slot==COOP_PLAYER_TWO &&
        gCoOpChaseActorPulses[1]==2);
    memset(queue,0,sizeof(queue));gCoOpDeferredOwners.Clear();gCoOpNodeOwners.Clear();
    p1.mHealth=100;p2.mHealth=0;CoOp_MissileAI(&missile,0);
    assert(gCoOpDeferredOwners.Get(chaseCameraScript,715).slot==COOP_PLAYER_ONE &&
        gCoOpNodeOwners.Get(715)==1 && gCoOpChaseActorPulses[0]==1);
    enqueueActorPulse=false;memset(queue,0,sizeof(queue));gCoOpDeferredOwners.Clear();gCoOpNodeOwners.Clear();
    scriptCalls=0;
    p2.mHealth=100;
    CScorpion scorpion;memset(&scorpion,0,sizeof(scorpion));
    scorpion.mPos.vx=10*4096;scorpion.field_BF8=1;
    CoOp_ScorpionAI(&scorpion,0);assert(observed==&p1 && !scorpion.hCurrentTarget.pWhatever);
    scorpion.field_BF8=2;CoOp_ScorpionAI(&scorpion,0);
    assert(observed==&p2 && scorpion.hCurrentTarget.pWhatever==&p2 && mech==&p1);
    scorpion.mPos.vx=100*4096;CoOp_ScorpionAI(&scorpion,0);
    assert(observed==&p1 && scorpion.hCurrentTarget.pWhatever==&p1);
    // Contact ownership follows the explicit collector, independently of enemy targeting.
    CPowerUp pickup={11,false};gCoOpPlayers.secondary=true;mech=&p2;
    assert(CoOp_Pickup(&pickup,0,&p2));gCoOpPlayers.secondary=false;mech=&p1;
    assert(pickupCalls==1 && observed==&p2 && CoOp_PendingCount()==1 && gCoOpPickupCount[1]==1);
    CoOpPlayerHandle owner=gCoOpDeferredOwners.Get(firstScript,42);
    assert(owner.slot==COOP_PLAYER_TWO && gCoOpPlayers.router.Resolve(owner));
    CoOp_SectionExecute(firstScript,42,0);
    assert(scriptCalls==1 && observed==&p2 && mech==&p1 && !gCoOpTriggerOwner);
    assert(gCoOpDeferredOwners.Get(childScript,43).slot==COOP_PLAYER_TWO);
    CoOp_SectionExecute(childScript,43,0);assert(scriptCalls==2 && observed==&p2);
    u16 raceEntry[]={195,0,199,0xFFFF};
    gCoOpRaceSectionsActive=true;
    scriptBank=&raceEntry;p2.field_1AC=1;
    gCoOpDeferredOwners.Record(raceEntry,304,gCoOpPlayers.router.Handle(COOP_PLAYER_TWO));
    CoOp_SectionExecute(raceEntry,304,0);
    assert(observed==&p2 && raceVisibilityTouches==1 && gCoOpRaceEntryScripts[1]==1);
    assert(gCoOpScriptMotion.Owns(gCoOpPlayers.router.Handle(COOP_PLAYER_TWO)));
    assert(!CoOp_EnemyAlive(gCoOpPlayers.router.Handle(COOP_PLAYER_ONE),&p1,0));
    assert(CoOp_EnemyAlive(gCoOpPlayers.router.Handle(COOP_PLAYER_TWO),&p2,0));
    // Both collision contacts were queued before the shared route began.
    unsigned callsBeforePartner=scriptCalls;
    gCoOpDeferredOwners.Record(raceEntry,71,gCoOpPlayers.router.Handle(COOP_PLAYER_ONE));
    CoOp_SectionExecute(raceEntry,71,0);
    assert(scriptCalls==callsBeforePartner && !p1.field_1AC);
    p2.field_1AC=0;gCoOpScriptMotion.Clear();scriptBank=0;
    --scriptCalls;
    gCoOpRaceSectionsActive=false;
    gCoOpPlayers.router.Withdraw(COOP_PLAYER_TWO);gCoOpPlayers.router.Publish(COOP_PLAYER_TWO,&p2);
    CoOp_SectionExecute(childScript,43,0);assert(scriptCalls==2 && gCoOpStaleScripts==1);
    assert(CoOp_Collector(gCoOpPlayers.router,&p1).slot==COOP_PLAYER_ONE);
    assert(CoOp_Collector(gCoOpPlayers.router,&p2).slot==COOP_PLAYER_TWO);
    // Unowned world script and singleplayer enemy retain original context/call count.
    CoOp_SectionExecute(childScript,999,0);assert(scriptCalls==3 && observed==&p1);
    // A P2-triggered level bank change is a shared-world action. It must not
    // swap the old P2 context back over the newly loaded level's state.
    u16 levelChange[]={305,0xFFFF};
    gCoOpDeferredOwners.Record(levelChange,777,gCoOpPlayers.router.Handle(COOP_PLAYER_TWO));
    unsigned contextsBefore=gCoOpPlayers.contextCalls;
    CoOp_SectionExecute(levelChange,777,0);
    assert(observed==&p1 && gCoOpPlayers.contextCalls==contextsBefore && !gCoOpTriggerOwner &&
        gCoOpSharedLevelChanges==1);
    memset(queue,0,sizeof(queue));gCoOpDeferredOwners.Clear();
    CoOpPlayerHandle fresh=gCoOpPlayers.router.Handle(COOP_PLAYER_TWO);
    gCoOpDeferredOwners.Record(childScript,123,fresh);scriptLetterbox=1;
    CoOp_SectionExecute(childScript,123,0);assert(letterbox==1 && gCoOpCinematicOwner.slot==COOP_PLAYER_TWO);
    gCoOpDeferredOwners.Clear();scriptLetterbox=-1;
    CoOp_SectionExecute(childScript,124,0);assert(observed==&p2 && letterbox==1); // later NPC pulse
    scriptLetterbox=0;CoOp_SectionExecute(childScript,125,0);
    assert(observed==&p2 && !letterbox && gCoOpCinematicOwner.slot==COOP_PLAYER_COUNT && !hudOffset);
    scriptLetterbox=-1;scriptCalls=3;
    assert(CoOp_InteractiveSceneOwner(true,true,3,5));
    assert(!CoOp_InteractiveSceneOwner(true,false,3,5));
    assert(!CoOp_InteractiveSceneOwner(true,true,5,5));
    assert(CoOp_CinematicOwnerActive(true,55,false));
    assert(!CoOp_CinematicOwnerActive(true,0,true));
    assert(!CoOp_CinematicOwnerActive(true,60,true));
    assert(!CoOp_CinematicOwnerActive(false,60,false));
    assert(CoOp_CinematicOwnerLive(gCoOpPlayers.router.Handle(COOP_PLAYER_ONE)));
    waitingPlayer[0]=true;
    assert(!CoOp_CinematicOwnerLive(gCoOpPlayers.router.Handle(COOP_PLAYER_ONE)));
    waitingPlayer[0]=false;
    interactiveScene=true;scriptLetterbox=30;
    gCoOpCinematicOwner=gCoOpPlayers.router.Handle(COOP_PLAYER_ONE);
    gCoOpDeferredOwners.Record(childScript,126,gCoOpCinematicOwner);
    CoOp_SectionExecute(childScript,126,0);
    assert(gCoOpCinematicOwner.slot==COOP_PLAYER_TWO);
    interactiveScene=false;scriptLetterbox=-1;scriptCalls=3;letterbox=0;
    gCoOpCinematicOwner=CoOpPlayerHandle();
    deferRaceFail=true;
    CoOp_SectionExecute(childScript,316,0);
    assert(scriptCalls==3 && !gCoOpCommandOwners);
    deferRaceFail=false;
    memset(queue,0,sizeof(queue));gCoOpDeferredOwners.Clear();
    CPowerUp p1Question={11,false};CoOp_Pickup(&p1Question,0,&p1);
    assert(gCoOpDeferredOwners.Get(firstScript,42).slot==COOP_PLAYER_ONE);
    CoOp_SectionExecute(firstScript,42,0);assert(scriptCalls==4 && observed==&p1);
    unsigned queued=CoOp_PendingCount();CoOp_Pickup(&p1Question,0,&p2);
    assert(CoOp_PendingCount()==queued && gCoOpDeferredOwners.Get(firstScript,42).slot==COOP_PLAYER_ONE);
    for(unsigned initiating=0;initiating<2;++initiating) {
        memset(queue,0,sizeof(queue));gCoOpDeferredOwners.Clear();gCoOpNodeOwners.Clear();
        gCoOpCinematicOwner=CoOpPlayerHandle();gCoOpCommandOwners=0;
        trailingFloor.mFlags=0;floorSections.Clear();floorSections.Remember(&trailingFloor);
        hideFloor=true;scriptLetterbox=30;
        gCoOpDeferredOwners.Record(firstScript,501,gCoOpPlayers.router.Handle((CoOpPlayerSlot)initiating));
        CoOp_SectionExecute(firstScript,501,0);
        assert(observedSectionOwner==(1u<<initiating) && !(trailingFloor.mFlags&1));
        assert(gCoOpNodeOwners.Get(43)==(1u<<initiating));
        gCoOpDeferredOwners.Clear();gCoOpNodeOwners.Clear();scriptLetterbox=-1;
        CoOp_SectionExecute(childScript,502,0);
        assert(observedSectionOwner==(1u<<initiating) && !(trailingFloor.mFlags&1));
        scriptLetterbox=0;CoOp_SectionExecute(childScript,503,0);
        assert(!(trailingFloor.mFlags&1) && !gCoOpCommandOwners);
        floorSections.Command(&trailingFloor,1u<<(1-initiating),false);
        assert(trailingFloor.mFlags&1);
    }
    hideFloor=false;scriptLetterbox=-1;
    puts("PASS cinematic section ownership: both roles retain partner floor through pickup/linked/delayed hides; both releases unload");
    // Execute the actual crane command through the production dispatcher,
    // including either actor while its partner is already dead/waiting.
    u16* craneTable[craneFixtureCount];
    for(unsigned ci=0;ci<craneFixtureCount;++ci)craneTable[ci]=craneBankWords+craneNodeOffsets[ci];
    u16* craneRoute=craneBankWords+craneRouteCommandWord;
    u16* craneExit=craneBankWords+craneExitCommandWord;
    SCommandPoint exitPoint={craneExit,156,0},routePoint={craneRoute,158,&exitPoint};
    craneNodes=craneTable;craneNodeCount=craneFixtureCount;cranePoints=&routePoint;
    strcpy((char*)craneResources,"L3A4_G");craneResources[10]=1;scriptBank=craneNodes;
    assert(CoOp_CraneExitForRoute(craneRoute,158)==craneExit);
    assert(!CoOp_CraneExitForRoute(craneRoute,70));
    u16 copiedRoute[11];memcpy(copiedRoute,craneRoute,sizeof(copiedRoute));
    assert(!CoOp_CraneExitForRoute(copiedRoute,158)); // stale/unregistered cursor
    craneResources[10]=0;assert(!CoOp_CraneExitForRoute(craneRoute,158));craneResources[10]=1;
    for(unsigned actor=0;actor<2;++actor)for(unsigned deadPartner=0;deadPartner<2;++deadPartner) {
        memset(queue,0,sizeof(queue));gCoOpDeferredOwners.Clear();gCoOpNodeOwners.Clear();
        p1.mHealth=p2.mHealth=100;waitingPlayer[0]=waitingPlayer[1]=false;
        if(deadPartner) {gCoOpPlayers.Get((CoOpPlayerSlot)(1-actor))->mHealth=0;waitingPlayer[1-actor]=true;}
        CoOpPlayerHandle collector=gCoOpPlayers.router.Handle((CoOpPlayerSlot)actor);
        gCoOpCinematicOwner=collector;gCoOpCraneAutoBank=0;
        gCoOpScriptMotion.Begin(collector,scriptBank,gCoOpPlayers.Get((CoOpPlayerSlot)actor)->mPos,0);
        letterbox=hudOffset=30;gCoOpDeferredOwners.Record(craneRoute,158,collector);
        unsigned calls=scriptCalls,contexts=gCoOpPlayers.contextCalls;
        CoOp_SectionExecute(craneRoute,158,0);
        assert(scriptCalls==calls+1 && observedScript==craneExit && observedScriptNode==156);
        assert(observedTriggerOwner==(1u<<actor) && observedSectionOwner==(1u<<actor));
        assert(observed==&p1 && gCoOpPlayers.contextCalls==contexts && !gCoOpPlayers.secondary);
        assert(!letterbox && !hudOffset && !gCoOpScriptMotion.active && gCoOpCraneAutoOwner==actor);
        CoOp_SectionExecute(craneRoute,158,0);assert(scriptCalls==calls+1); // duplicate pulse
    }
    // An unowned late pulse must use the survivor, never the dead P1 model.
    gCoOpCraneAutoBank=0;gCoOpDeferredOwners.Clear();gCoOpCinematicOwner=CoOpPlayerHandle();
    p1.mHealth=0;waitingPlayer[0]=true;p2.mHealth=100;waitingPlayer[1]=false;
    CoOp_SectionExecute(craneRoute,158,0);
    assert(observedScript==craneExit && observedTriggerOwner==2 && gCoOpCraneAutoOwner==1);
    // Original singleplayer dispatch retains the native crane motion.
    gCoOpSession.enabled=false;gCoOpDeferredOwners.Clear();gCoOpCinematicOwner=CoOpPlayerHandle();
    CoOp_SectionExecute(craneRoute,158,0);assert(observedScript==craneRoute && observedScriptNode==158);
    gCoOpSession.enabled=true;craneNodes=0;cranePoints=0;craneNodeCount=0;scriptBank=0;
    p1.mHealth=p2.mHealth=100;waitingPlayer[0]=waitingPlayer[1]=false;
    // Dead-owner cleanup must clear presentation before forgetting its handle.
    for(unsigned deadActor=0;deadActor<2;++deadActor) {
        gCoOpCinematicOwner=gCoOpPlayers.router.Handle((CoOpPlayerSlot)deadActor);
        letterbox=hudOffset=30;CoOp_ReleaseDeadCinematic();assert(letterbox==30);
        waitingPlayer[deadActor]=true;CoOp_ReleaseDeadCinematic();
        assert(!letterbox && !hudOffset && gCoOpCinematicOwner.slot==COOP_PLAYER_COUNT);
        waitingPlayer[deadActor]=false;
    }
    puts("PASS actual crane dispatch: both actors, dead partner, unowned survivor, shared native exit once, original singleplayer, dead cinematic cleanup");
    // The next bank loaded in the live failure, but neither its authored skip
    // destination nor the retail post-frame handoff ran. Exercise that second
    // stage against L3A5's actual route and marker links, before any fall.
    u16* arrivalTable[arrivalFixtureCount];
    for(ci=0;ci<arrivalFixtureCount;++ci)arrivalTable[ci]=arrivalBankWords+arrivalNodeOffsets[ci];
    SCommandPoint arrivalPoint={arrivalBankWords+arrivalCommandWord,22,0};
    craneNodes=arrivalTable;craneNodeCount=arrivalFixtureCount;cranePoints=&arrivalPoint;
    strcpy((char*)craneResources,"L3A5_G");craneResources[10]=1;
    u16** p1Cursor=reinterpret_cast<u16**>(reinterpret_cast<u8*>(&p1)+0x1B8);
    *p1Cursor=arrivalPoint.pCommands+2;
    gCoOpCraneArrivalPending=true;gCoOpCraneArrivalRequested=false;
    p1.field_1AC=1;p1.field_1A8=0;nativeSkipFlag=0;
    CoOp_CraneArrivalFrame(&p1);assert(!nativeSkipFlag && !gCoOpCraneArrivalRequested);
    p1.field_1A8=26;*p1Cursor=arrivalPoint.pCommands+9;
    CoOp_CraneArrivalFrame(&p2);assert(!nativeSkipFlag); // P2 is the hidden partner here
    unsigned arrivalCount=gCoOpCraneArrivalCount;
    CoOp_CraneArrivalFrame(&p1);assert(nativeSkipFlag==1 && gCoOpCraneArrivalRequested);
    CoOp_CraneArrivalFrame(&p1);assert(gCoOpCraneArrivalPending && gCoOpCraneArrivalCount==arrivalCount);
    p1.field_1AC=0;CoOp_CraneArrivalFrame(&p1);
    assert(gCoOpCraneArrivalPending); // synth input ended; retail frame still owns skip
    nativeSkipFlag=0;CoOp_CraneArrivalFrame(&p1); // retail camera/body handoff consumed it
    assert(!gCoOpCraneArrivalPending && !gCoOpCraneArrivalRequested && gCoOpCraneArrivalCount==arrivalCount+1);
    CoOp_CraneArrivalFrame(&p1);assert(gCoOpCraneArrivalCount==arrivalCount+1);
    gCoOpCraneArrivalPending=true;gCoOpCraneArrivalRequested=false;p1.field_1AC=1;
    *p1Cursor=craneRoute;CoOp_CraneArrivalFrame(&p1);assert(!nativeSkipFlag); // old bank cursor
    *p1Cursor=arrivalPoint.pCommands+9;craneResources[10]=0;
    CoOp_CraneArrivalFrame(&p1);assert(!nativeSkipFlag);craneResources[10]=1;
    gCoOpSession.enabled=false;CoOp_CraneArrivalFrame(&p1);assert(!gCoOpCraneArrivalPending);
    gCoOpSession.enabled=true;p1.field_1AC=0;p1.field_1A8=0;craneNodes=0;cranePoints=0;craneNodeCount=0;
    puts("PASS post-crane native handoff: installed skip destination/linked facing, initialization order, correct body/bank, post-frame flag consumption, once only, singleplayer unchanged");
    for(unsigned aimer=0;aimer<2;++aimer) {
        CPlayer* player=aimer?&p2:&p1;player->field_8EA=1;
        gCoOpCinematicOwner=CoOpPlayerHandle();letterbox=hudOffset=30;
        CoOp_EndAimPresentation(player);assert(!letterbox && !hudOffset && player->field_8EA);
        gCoOpCinematicOwner=gCoOpPlayers.router.Handle((CoOpPlayerSlot)aimer);
        letterbox=hudOffset=30;CoOp_EndAimPresentation(player);assert(letterbox==30 && hudOffset==30);
        player->field_8EA=0;gCoOpCinematicOwner=CoOpPlayerHandle();
    }
    gCoOpPlayers.router.Withdraw(COOP_PLAYER_TWO);enemy.field_208=456;
    p1.field_8EA=1;letterbox=hudOffset=30;CoOp_EndAimPresentation(&p1);
    assert(letterbox==30 && hudOffset==30);p1.field_8EA=0;
    puts("PASS aim presentation: both players keep local aim/camera state without shared letterbox; authored scenes and singleplayer preserved");
    CoOp_ThugAI(&enemy,0);assert(observed==&p1 && enemy.field_208==456);
    liz.field_208=789;CoOp_LizManAI(&liz,0);assert(observed==&p1 && liz.field_208==789);
    CPowerUp single={8,false};CoOp_Pickup(&single,0,&p1);assert(observed==&p1);
    puts("PASS production pickup/enemy/script wrappers: exact collector, deferred and linked ownership, stale rejection, nearest-visible retarget, one native AI call, restored context, singleplayer fallback");
    return 0;
}
