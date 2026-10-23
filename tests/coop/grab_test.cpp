#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "router_test_types.h"
#include "coop/core/coop_router.h"
struct SHandle {void* body;unsigned generation;};
struct CBaddy:CSuper {int field_208;u32 field_218;union {u32 bothFlags;} field_31C;u8 venomStorage[0x300];};
static CPlayer players[2],*mech;
static CBaddy enemies[2];static bool live[2];static unsigned calls,releaseCalls;
static int action;static unsigned wrapDeaths;static int wrapEffect;
static void*& Wrap(CBaddy* boss){return *reinterpret_cast<void**>((u8*)boss+0x3E4);}
static void __fastcall EndWrap(void* wrap,void*) {assert(wrap==&wrapEffect);++wrapDeaths;}
static struct Registry {
    CoOpPlayerRouter roster;
    CPlayer* Get(CoOpPlayerSlot slot){return &players[slot];}
    CoOpPlayerRouter& Router(){return roster;}
    bool InSecondaryContext(){return false;}
    bool WithSecondaryContext(void(*fn)(void*),void* arg){CPlayer* previous=mech;mech=&players[1];fn(arg);mech=previous;return true;}
} gCoOpPlayers;
static CBaddy* __cdecl Resolve(const SHandle* h) {
    for(unsigned i=0;i<2;++i)if(h->body==&enemies[i] && live[i])return &enemies[i];return 0;
}
static SHandle __cdecl Handle(void* enemy){SHandle h={enemy,1};return h;}
static void __fastcall Release(CPlayer* p,void*,int anim,int mode,int target) {
    assert(p==mech && (anim==172 || anim==176) && mode==0 && target==-1);++releaseCalls;p->mAnim=anim;
}
static void __fastcall Native(CBaddy* enemy,void*) {
    ++calls;
    if(action==1){mech->field_E1C=enemy->mType==313?0x20000000:0x10000000;
        if(enemy->mType==313){*reinterpret_cast<u32*>((u8*)mech+0x554)=0x004F0CA0;enemy->field_218|=0x4000;}}
    if(action==2)enemy->field_31C.bothFlags=enemy->mType==313?0x20:0x19;
    if(action==3 || action==4){*reinterpret_cast<u32*>((u8*)mech+0x554)=action==3?0x004F0CA0:0x004F0D70;enemy->field_218|=0x4000;}
    if(action==5){*reinterpret_cast<u32*>((u8*)mech+0x554)=0x004F0A70;
        enemy->field_218|=0x4000;enemy->field_194|=0x4800;Wrap(enemy)=&wrapEffect;mech->mAnim=183;}
    if(action==6){*reinterpret_cast<u32*>((u8*)mech+0x554)=0x123456;
        mech->field_E1C=0x800000;mech->mAnim=176;enemy->field_218&=~0x4000;
        enemy->field_194&=~0x4800;EndWrap(Wrap(enemy),0);Wrap(enemy)=0;}
}
static bool NativeDamage(CBaddy* enemy,unsigned damage) {
    // Audited native Venom Hit early guard at 004EB83A-004EB843. HP is
    // subtracted by Hit only AFTER passing this guard (004EB8E0).
    if(enemy->field_218&0x4000)return false;
    enemy->mHealth-=damage;return true;
}
#include "grab_runtime_test.h"
int main() {
    unsigned victim;
    for(victim=0;victim<2;++victim)for(unsigned kind=0;kind<2;++kind) {
        memset(players,0,sizeof(players));memset(enemies,0,sizeof(enemies));
        gCoOpPlayers.roster.Clear();gCoOpPlayers.roster.Publish(COOP_PLAYER_ONE,&players[0]);gCoOpPlayers.roster.Publish(COOP_PLAYER_TWO,&players[1]);
        for(unsigned i=0;i<2;++i){gCoOpGrab[i]=CoOpGrabLease();players[i].mHealth=100;players[i].field_E1C=1;live[i]=true;}
        mech=&players[victim];*reinterpret_cast<u32*>((u8*)mech+0x554)=0x123456;
        CBaddy* enemy=&enemies[kind];enemy->mType=kind?313:317;enemy->mHealth=100;enemy->field_31C.bothFlags=kind?0x100:0x10;
        CoOpEnemyCall call={enemy,reinterpret_cast<u32>(&Native)};
        action=1;CoOp_RunEnemy(&call);assert(gCoOpGrab[victim].state && !gCoOpGrab[1-victim].state);
        CoOpPlayerHandle other=gCoOpPlayers.roster.Handle((CoOpPlayerSlot)(1-victim));
        assert(CoOp_GrabTarget(enemy,other).slot==victim);
        action=0;CoOp_RunEnemy(&call);assert(mech->field_E1C==gCoOpGrab[victim].state);
        enemy->field_218|=0xA180; // unrelated attack/visibility/effect flags
        if(kind)assert(!NativeDamage(enemy,10) && enemy->mHealth==100);
        action=2;CoOp_RunEnemy(&call);assert(!gCoOpGrab[victim].state && mech->field_E1C==0x800000 && mech->mAnim==172);
        assert(players[1-victim].field_E1C==1 && players[1-victim].mHealth==100);
        assert(*reinterpret_cast<u32*>((u8*)mech+0x554)==0x123456);
        assert((enemy->field_218&0xA180)==0xA180);
        if(kind)assert(NativeDamage(enemy,10) && enemy->mHealth==90);
        enemy->mHealth=100;
        action=0;CoOp_RunEnemy(&call);assert(!gCoOpGrab[victim].state);
        // Destroyed grabbers resolve to null; no stale enemy access.
        action=1;enemy->field_31C.bothFlags=kind?0x100:0x10;CoOp_RunEnemy(&call);
        live[kind]=false;action=0;CoOp_ReleaseInterruptedGrab(mech,gCoOpGrab[victim]);
        assert(!gCoOpGrab[victim].state && mech->field_E1C==0x800000);
        live[kind]=true;action=1;CoOp_RunEnemy(&call);mech->mHealth=0;
        assert(CoOp_GrabTarget(enemy,other).slot==1-victim);
        assert(!gCoOpGrab[victim].state && mech->mHealth==0);
        assert(*reinterpret_cast<u32*>((u8*)mech+0x554)==0x123456);
    }
    assert(releaseCalls==8);
    // Venom installs the pin before the player's next animation command
    // assigns its held state. All three native pin variants acquire a lease.
    for(unsigned pin=3;pin<=5;++pin)for(victim=0;victim<2;++victim) {
        for(unsigned i=0;i<2;++i){gCoOpGrab[i]=CoOpGrabLease();players[i].mHealth=100;players[i].field_E1C=1;players[i].field_1AC=0;live[i]=true;}
        mech=&players[victim];*reinterpret_cast<u32*>((u8*)mech+0x554)=0x123456+victim;
        CBaddy* enemy=&enemies[0];enemy->mType=313;enemy->mHealth=100;enemy->field_31C.bothFlags=pin==5?0x40:0x100;
        enemy->field_194=0x100;Wrap(enemy)=0; // unrelated render flag survives
        CoOpEnemyCall call={enemy,reinterpret_cast<u32>(&Native)};
        action=pin;CoOp_RunEnemy(&call);
        assert(gCoOpGrab[victim].state==0x20000000 && mech->field_E1C==1);
        action=0;CoOp_RunEnemy(&call);assert(gCoOpGrab[victim].state);
        players[victim].field_E1C=pin==5?0x800000:0x20000000;
        mech=&players[0];CoOp_GrabFrame();assert(gCoOpGrab[victim].state);
        assert(!NativeDamage(enemy,7));
        enemy->field_31C.bothFlags=0x400; // native web-stagger phase
        // No enemy AI call occurs after this interruption.
        CoOp_GrabFrame();assert(!gCoOpGrab[victim].state && players[victim].field_E1C==0x800000);
        assert(*reinterpret_cast<u32*>((u8*)&players[victim]+0x554)==0x123456+victim);
        assert(players[1-victim].field_E1C==1 && mech==&players[0]);
        assert(players[victim].mAnim==(pin==5?176:172));
        assert(enemy->field_194==0x100 && !Wrap(enemy));
        assert(NativeDamage(enemy,7) && enemy->mHealth==93);
        unsigned deaths=wrapDeaths;CoOp_GrabFrame();assert(wrapDeaths==deaths);
    }
    assert(wrapDeaths==2);
    // Native completion must remain native: no repeated animation or effect Die.
    for(victim=0;victim<2;++victim) {
        CBaddy* enemy=&enemies[0];enemy->field_31C.bothFlags=0x40;enemy->field_194=0x100;
        mech=&players[victim];mech->field_E1C=1;
        *reinterpret_cast<u32*>((u8*)mech+0x554)=0x123456;
        CoOpEnemyCall call={enemy,reinterpret_cast<u32>(&Native)};
        action=5;CoOp_RunEnemy(&call);assert(gCoOpGrab[victim].state);
        unsigned releases=releaseCalls,deaths=wrapDeaths;
        action=6;CoOp_RunEnemy(&call);
        assert(!gCoOpGrab[victim].state && releaseCalls==releases && wrapDeaths==deaths+1);
        assert(mech->mAnim==176 && enemy->field_194==0x100 && NativeDamage(enemy,3));
    }
    // Death during the third hold cleans the callback/effect without reviving.
    for(victim=0;victim<2;++victim) {
        CBaddy* enemy=&enemies[0];enemy->mHealth=100;enemy->field_31C.bothFlags=0x40;
        enemy->field_194=0x4100; // 4000 is pre-existing, not owned by this hold
        mech=&players[victim];mech->mHealth=100;mech->field_E1C=1;
        *reinterpret_cast<u32*>((u8*)mech+0x554)=0x123456;
        CoOpEnemyCall call={enemy,reinterpret_cast<u32>(&Native)};
        action=5;CoOp_RunEnemy(&call);assert(gCoOpGrab[victim].state);
        unsigned releases=releaseCalls;mech->mHealth=0;
        mech=&players[0];CoOp_GrabFrame();
        assert(!gCoOpGrab[victim].state && players[victim].mHealth==0 && releaseCalls==releases);
        assert(*reinterpret_cast<u32*>((u8*)&players[victim]+0x554)==0x123456);
        assert(!Wrap(enemy) && enemy->field_194==0x4100 && NativeDamage(enemy,3));
        players[victim].mHealth=100;
    }
    // Two different enemies can hold two players in the SAME native state.
    // Releasing one must neither release nor reassign the other's lease.
    for(victim=0;victim<2;++victim) {
        players[victim].field_E1C=1;players[victim].mHealth=100;live[victim]=true;
        enemies[victim].mType=317;enemies[victim].mHealth=100;enemies[victim].field_31C.bothFlags=0x10;
        mech=&players[victim];CoOpEnemyCall call={&enemies[victim],reinterpret_cast<u32>(&Native)};
        action=1;CoOp_RunEnemy(&call);
    }
    mech=&players[0];enemies[0].field_31C.bothFlags=0x19;CoOp_GrabFrame();
    assert(!gCoOpGrab[0].state && players[0].field_E1C==0x800000);
    assert(gCoOpGrab[1].state && players[1].field_E1C==0x10000000);
    live[1]=false;CoOp_GrabFrame();assert(!gCoOpGrab[1].state && players[1].field_E1C==0x800000);
    // A replacement player at the same address does not inherit an old lease.
    live[0]=true;players[0].field_E1C=1;mech=&players[0];enemies[0].field_31C.bothFlags=0x10;
    CoOpEnemyCall stale={&enemies[0],reinterpret_cast<u32>(&Native)};action=1;CoOp_RunEnemy(&stale);
    gCoOpPlayers.roster.Withdraw(COOP_PLAYER_ONE);gCoOpPlayers.roster.Publish(COOP_PLAYER_ONE,&players[0]);
    unsigned releasesBefore=releaseCalls;CoOp_GrabFrame();assert(!gCoOpGrab[0].state && releaseCalls==releasesBefore);
    // Never thaw unowned holds, change health, or interrupt a synthesized route.
    assert(players[0].field_E1C==0x10000000 && players[0].mHealth==100);
    players[0].field_E1C=1;action=3;enemies[0].mType=313;CoOp_RunEnemy(&stale);
    assert(gCoOpGrab[0].state);players[0].field_1AC=1;
    players[0].field_E1C=0x20000000;enemies[0].field_31C.bothFlags=0x20;
    CoOp_GrabFrame();assert(!gCoOpGrab[0].state && releaseCalls==releasesBefore);
    assert(*reinterpret_cast<u32*>((u8*)&players[0]+0x554)!=0x004F0CA0);
    assert(!(enemies[0].field_218&0x4000));
    // Unowned invulnerability must not be removed by the general frame check.
    players[0].field_1AC=0;enemies[0].field_218|=0x4000;CoOp_GrabFrame();
    assert(!NativeDamage(&enemies[0],10));enemies[0].field_218&=~0x4000;
    // If another valid owner still holds the same enemy, transfer the owned
    // flag rather than removing protection from a live restraint.
    for(victim=0;victim<2;++victim){players[victim].field_E1C=1;players[victim].mHealth=100;
        *reinterpret_cast<u32*>((u8*)&players[victim]+0x554)=0x123456;
        mech=&players[victim];enemies[0].field_31C.bothFlags=0x100;
        action=3;CoOp_RunEnemy(&stale);}
    players[0].field_E1C=1;*reinterpret_cast<u32*>((u8*)&players[0]+0x554)=0x123456;
    mech=&players[0];CoOp_GrabFrame();
    assert(!gCoOpGrab[0].state && gCoOpGrab[1].state && (enemies[0].field_218&0x4000));
    enemies[0].field_31C.bothFlags=0x400;CoOp_GrabFrame();
    assert(!gCoOpGrab[1].state && NativeDamage(&enemies[0],10));
    puts("PASS independent same-state grabs, all three Venom restraints including captured airborne wrap, native wrap exit, interruption restores damage without removing live/unowned immunity, shared-owner transfer and stale/scripted protection");return 0;
}
