// Co-op / player: Construct, update, carry, and recover each independent retail CPlayer.
#pragma once
#include "coop_life.h"
static CoOpLife gCoOpLife;
static bool gCoOpLifeInstalled;
static CoOpPlayerHandle gCoOpLifeHandles[2];
static CVector gCoOpSafeGround[2];
static short gCoOpSafeYaw[2];
static bool gCoOpHasSafeGround[2];
static DWORD gCoOpSafeSample[2];
static int gCoOpLifeCheckpoint=-1,gCoOpLifeSection=-1;
static bool gCoOpRevivePrimaryNextLevel;
// Carry both players' health, armor and web inventory into the next bank.
static void CoOp_LifeRememberLevelChange() {
    if(!gCoOpSession.enabled) {gCoOpRevivePrimaryNextLevel=false;return;}
    // A living P2 may complete a segment while P1 is waiting. The next
    // segment is a checkpoint: its freshly constructed P1 must rejoin there.
    if(gCoOpSession.enabled && gCoOpLife.active &&
       gCoOpLife.slot[0].waiting && !gCoOpLife.slot[1].waiting)
        gCoOpRevivePrimaryNextLevel=true;
}
// Restore death-only default web, health and armor values for one player.
static void CoOp_LifeResetInventory(CPlayer* player) {
    CoOpPlayerCarry fresh;fresh.Capture(player);fresh.ResetAfterDeath();
    reinterpret_cast<void (__fastcall*)(CPlayer*,void*,bool)>(0x004BAEC0)(player,0,false);
    fresh.Apply(player);
}
// Reattach P1 to retail update after a co-op waiting state ends.
static void CoOp_LifeResumePrimary(CPlayer* player) {
    if(gCoOpSession.enabled && player && player==gCoOpPlayers.Get(COOP_PLAYER_ONE) &&
       gCoOpSession.primaryCarry.valid) {
        reinterpret_cast<void (__fastcall*)(CPlayer*,void*,bool)>(0x004BAEC0)(player,0,gCoOpSession.primaryCarry.armor);
        gCoOpSession.primaryCarry.Apply(player);gCoOpSession.primaryCarry.Clear();
    }
    if(!gCoOpRevivePrimaryNextLevel || !gCoOpSession.enabled ||
       !gCoOpSession.pending || !player ||
       player!=gCoOpPlayers.Get(COOP_PLAYER_ONE))return;
    if(player->mHealth<=0)CoOp_LifeResetInventory(player);
    player->mFlags&=~1;
    player->mCBodyFlags&=~0x40;
    gCoOpRevivePrimaryNextLevel=false;
}
// Enable co-op life handling only when both registered CPlayers exist.
static bool CoOp_LifeActive(){return gCoOpLifeInstalled && gCoOpPlayers.Get(COOP_PLAYER_ONE) && gCoOpPlayers.Get(COOP_PLAYER_TWO);}
// Map an actual CPlayer pointer to P1, P2 or an invalid slot.
static unsigned CoOp_LifeSlot(CPlayer* p){return p==gCoOpPlayers.Get(COOP_PLAYER_ONE)?0:p==gCoOpPlayers.Get(COOP_PLAYER_TWO)?1:2;}
// Retail geometry/orientation helpers share transient GTE registers.
class CoOpLifeMathScope {
    u8 saved[0x100];
public:
    // Initialize co-op life math scope with no active retail ownership.
    CoOpLifeMathScope(){memcpy(saved,reinterpret_cast<void*>(0x00610B20),sizeof(saved));}
    // Release the scoped state owned by this object.
    ~CoOpLifeMathScope(){memcpy(reinterpret_cast<void*>(0x00610B20),saved,sizeof(saved));}
};
// Synchronize each player's health/death tracker with the live retail roster.
static void CoOp_LifeSync() {
    CoOpLifeMathScope math;
    bool active=CoOp_LifeActive();
    if(!active){gCoOpLife.Clear();return;}
    if(!gCoOpLife.active) {gCoOpLife.Clear();gCoOpLifeCheckpoint=*reinterpret_cast<int*>(0x0055804C);gCoOpLifeSection=-1;}
    gCoOpLife.active=true;
    for(unsigned i=0;i<2;++i) {
        CoOpPlayerHandle h=gCoOpPlayers.Router().Handle((CoOpPlayerSlot)i);
        if(h.generation!=gCoOpLifeHandles[i].generation || h.slot!=gCoOpLifeHandles[i].slot) {
            gCoOpLife.slot[i]=CoOpLifeSlot();gCoOpHasSafeGround[i]=false;gCoOpSafeSample[i]=0;gCoOpLifeHandles[i]=h;
            CPlayer* initial=gCoOpPlayers.Get((CoOpPlayerSlot)i);CVector point;
            if(initial && initial->mHealth>0 && CoOp_FindSpawn(initial,point)) {
                gCoOpSafeGround[i]=point;gCoOpSafeYaw[i]=initial->mAngles.vy;gCoOpHasSafeGround[i]=true;
            }
        }
        CPlayer* p=gCoOpPlayers.Get((CoOpPlayerSlot)i);
        if(p->mHealth<=0) {if(!gCoOpLife.slot[i].waiting)gCoOpLife.Die(i);p->mFlags|=1;p->mCBodyFlags&=~0x40;}
    }
}
// Report whether this player is waiting for the next checkpoint.
static bool CoOp_LifeWaiting(unsigned slot){return gCoOpLife.Waiting(slot);}
// Handle one player's death without ending the level while the partner lives.
static void CoOp_LifeDeath(CPlayer* p) {
    unsigned i=CoOp_LifeSlot(p);if(i>=2)return;
    gCoOpLife.active=true;gCoOpLife.Die(i);p->mHealth=0;
    p->mFlags|=1;p->mCBodyFlags&=~0x40;
    // A dead actor cannot finish an authored input route. Release the scene
    // latch so its living partner can continue toward the next checkpoint.
    if(p->field_1AC && *reinterpret_cast<i32*>(0x00660F80)) {
        p->field_1AC=0;
        *reinterpret_cast<u8*>(reinterpret_cast<u8*>(p)+0x1B4)=0;
        *reinterpret_cast<i32*>(0x00660F80)=0;
        *reinterpret_cast<i32*>(0x0060F76C)=0;
    }
}
static unsigned gCoOpFallEvents[2];
// Charge a fall to its actual player and enter checkpoint wait at zero health.
static void CoOp_LifeFall(CPlayer* p) {
    CoOp_LifeSync();unsigned i=CoOp_LifeSlot(p);if(i>=2)return;
    if(!gCoOpLife.slot[i].waiting && !gCoOpLife.slot[i].recover)++gCoOpFallEvents[i];
    p->mHealth=(short)gCoOpLife.Fall(i,p->mHealth,p->mMaxHealth);
    if(p->mHealth<=0)CoOp_LifeDeath(p);
}
static void __fastcall CoOp_PlayerDie(CPlayer* p,void*) {
    if(!CoOp_LifeActive() || CoOp_LifeSlot(p)>=2) {
        if(!reinterpret_cast<int (__fastcall*)(CBody*,void*)>(0x00460700)(p,0)) {p->mCBodyFlags|=0x40;p->mFlags|=1;}
        return;
    }
    CoOp_LifeSync();unsigned i=CoOp_LifeSlot(p);
    // The retail Die virtual is also used by kill volumes. A descending player
    // still carrying health is a fall; lethal combat enters SwitchToDeathMode.
    if(p->mHealth>0 && (p->mVel.vy>0 || p->field_E1C==4)) {
        p->mHealth=(short)gCoOpLife.Fall(i,p->mHealth,p->mMaxHealth);
        if(!p->mHealth) CoOp_LifeDeath(p);
    } else CoOp_LifeDeath(p);
}
static void __fastcall CoOp_DeathMode(CPlayer* p,void*,bool immediate) {
    if(!CoOp_LifeActive() || CoOp_LifeSlot(p)>=2) {
        reinterpret_cast<void (__fastcall*)(CPlayer*,void*,bool)>(0x004BDFF0)(p,0,immediate);return;
    }
    CoOp_LifeSync();CoOp_LifeDeath(p);
}
static void __fastcall CoOp_ScriptDeath(CPlayer* p,void*,bool immediate) {
    if(!CoOp_LifeActive() || CoOp_LifeSlot(p)>=2) {
        reinterpret_cast<void (__fastcall*)(CPlayer*,void*,bool)>(0x004BDFF0)(p,0,immediate);return;
    }
    if(p->mHealth>0 && (p->mVel.vy>0 || p->field_E1C==4))CoOp_LifeFall(p);
    else {CoOp_LifeSync();CoOp_LifeDeath(p);}
    // Die is terminal for this authored sequence. Release its private scripted
    // control latch and shared letterbox so recovery is not held by that scene.
    *reinterpret_cast<u8*>(reinterpret_cast<u8*>(p)+0x1AC)=0;
    *reinterpret_cast<u8*>(reinterpret_cast<u8*>(p)+0x1B4)=0;
    *reinterpret_cast<i32*>(0x00660F80)=0;
    *reinterpret_cast<i32*>(0x0060F76C)=0;
}
static int __fastcall CoOp_GameOver(CBody* primary,void*) {
    if(gCoOpRevivePrimaryNextLevel && gCoOpSession.enabled && gCoOpSession.pending)
        return 0; // P2 completed the old level; wait for the new checkpoint body
    if(!CoOp_LifeActive()) return reinterpret_cast<int (__fastcall*)(CBody*,void*)>(0x00460700)(primary,0);
    CoOp_LifeSync();return gCoOpLife.BothDead()?1:0;
}
// Update life restore on the selected CPlayer's independent lifecycle.
static void CoOp_LifeRestore(void* argument) {
    CoOpLifeMathScope math;
    unsigned i=*static_cast<unsigned*>(argument);
    CPlayer* p=gCoOpPlayers.Get((CoOpPlayerSlot)i);
    bool revival=gCoOpLife.slot[i].waiting && gCoOpLife.slot[i].revive;
    CVector point;short yaw=0;bool found=false;
    CPlayer* other=gCoOpPlayers.Get((CoOpPlayerSlot)(1-i));
    if(revival && other && other->mHealth>0) {
        found=CoOp_FindSpawn(other,point);yaw=other->mAngles.vy;
    } else if(gCoOpHasSafeGround[i]) {
        point=gCoOpSafeGround[i];yaw=gCoOpSafeYaw[i];
        i32 ground=reinterpret_cast<i32 (__cdecl*)(CVector*,i32,i32,CBody**)>(0x004E6840)(&point,16,160,0);
        found=ground!=-1 && ground>=point.vy && ground<=point.vy+160*4096;
    }
    if(!found && !revival && other && other->mHealth>0) {found=CoOp_FindSpawn(other,point);yaw=other->mAngles.vy;}
    if(!found) return; // hold recovery until there is a supported place
    if(revival)CoOp_LifeResetInventory(p);
    p->mAngles.vx=p->mAngles.vz=0;p->mAngles.vy=yaw;
    p->mPos=point;p->mVel.vx=p->mVel.vy=p->mVel.vz=0;
    p->mAcc.vx=p->mAcc.vy=p->mAcc.vz=0;
    p->mCBodyFlags&=~0x40;p->mFlags&=~1;
    p->field_A8.vx=p->field_A8.vz=0;p->field_A8.vy=-4096;
    *reinterpret_cast<u8*>(reinterpret_cast<u8*>(p)+0x8E8)=0;
    *reinterpret_cast<u8*>(reinterpret_cast<u8*>(p)+0x8E9)=0;
    memset(&p->mTransform,0,sizeof(p->mTransform));
    p->mTransform.m[0][0]=p->mTransform.m[1][1]=p->mTransform.m[2][2]=4096;
    short angles[3]={0,yaw,0};
    reinterpret_cast<void (__fastcall*)(CPlayer*,void*,short*)>(0x004B9E50)(p,0,angles);
    reinterpret_cast<void (__fastcall*)(CPlayer*,void*)>(0x004BE4B0)(p,0);
    reinterpret_cast<void (__fastcall*)(CPlayer*,void*,int)>(0x004BF720)(p,0,0);
    CCamera* camera=*reinterpret_cast<CCamera**>(0x0056F3B8);
    if(camera) reinterpret_cast<void (__fastcall*)(CCamera*,void*)>(0x00416AA0)(camera,0);
    gCoOpLife.Recovered(i);gCoOpSafeGround[i]=point;gCoOpSafeYaw[i]=yaw;gCoOpHasSafeGround[i]=true;
}
// Apply a player's life gate before calling native body AI.
static bool CoOp_LifeBefore(CPlayer* p) {
    if(!CoOp_LifeActive())return true;
    CoOp_LifeSync();unsigned i=CoOp_LifeSlot(p);if(i>=2)return true;
    // Recovery waits for letterbox to end. If a live scripted actor also
    // stops ticking, the scene can never end (L3A4's crane is one example).
    // Fall() already prevents a second health charge while recover is set.
    return !gCoOpLife.slot[i].waiting &&
        (!gCoOpLife.slot[i].recover ||
         (p->field_1AC && *reinterpret_cast<i32*>(0x00660F80)));
}
// Advance per-player death, waiting and checkpoint recovery once per frame.
static void CoOp_LifeFrame() {
    CoOpLifeMathScope math;
    CoOp_LifeSync();if(!gCoOpLife.active)return;
    int checkpoint=*reinterpret_cast<int*>(0x0055804C);
    if(checkpoint!=gCoOpLifeCheckpoint) {gCoOpLifeCheckpoint=checkpoint;if(checkpoint>=0 && checkpoint!=0xFFFF)gCoOpLife.Checkpoint();}
    if(gCoOpLife.BothDead())return;
    bool paused=*reinterpret_cast<i32*>(0x00ADB4D8)!=0;
    if(paused || *reinterpret_cast<i32*>(0x00660F80))return;
    for(unsigned i=0;i<2;++i) {
        CPlayer* p=gCoOpPlayers.Get((CoOpPlayerSlot)i);
        if(gCoOpLife.slot[i].recover || gCoOpLife.slot[i].revive) {
            if(i) gCoOpPlayers.WithSecondaryContext(CoOp_LifeRestore,&i);else CoOp_LifeRestore(&i);
        }
        if(!gCoOpLife.slot[i].waiting && !gCoOpLife.slot[i].recover && (p->mCollision&2) &&
            (DWORD)(GetTickCount()-gCoOpSafeSample[i])>=350) {
            gCoOpSafeSample[i]=GetTickCount();CVector point;
            if(CoOp_FindSpawn(p,point)){gCoOpSafeGround[i]=point;gCoOpSafeYaw[i]=p->mAngles.vy;gCoOpHasSafeGround[i]=true;}
        }
    }
}
// Record section progression for checkpoint respawn of either player.
static void CoOp_LifeSection(const char* prefix,bool visible,unsigned owners) {
    if(!visible || !gCoOpLife.active || !owners)return;
    unsigned living=(gCoOpLife.slot[0].waiting?0:1)|(gCoOpLife.slot[1].waiting?0:2);
    if(!(owners&living))return;
    const char* names[]={"Start_","First_","Second_","Third_","Fourth_","Fifth_","Sixth_","Seventh_","Eight_"};
    for(int i=0;i<9;++i)if(!strcmp(prefix,names[i]) && i>gCoOpLifeSection) {
        gCoOpLifeSection=i;gCoOpLife.Checkpoint();return;
    }
}
// Update legacy death end on the selected CPlayer's independent lifecycle.
static int CoOp_LegacyDeathEnd(CPlayer* p) {
    if(!CoOp_LifeActive())return 0;
    CoOp_LifeSync();unsigned i=CoOp_LifeSlot(p);if(i>=2)return 0;
    if(p->mHealth>0) p->mHealth=(short)gCoOpLife.Fall(i,p->mHealth,p->mMaxHealth);
    if(p->mHealth<=0) CoOp_LifeDeath(p);
    return 1;
}
// Update death end bridge on the selected CPlayer's independent lifecycle.
static void __declspec(naked) CoOp_DeathEndBridge() {
    __asm {
        pushad
        push esi
        call CoOp_LegacyDeathEnd
        add esp,4
        test eax,eax
        popad
        jz original
        push 004B6E8Ch
        ret
    original:
        mov dword ptr ds:[005FB1B0h],eax
        push 004B28B3h
        ret
    }
}
// Install install life hooks at the supported retail call site, preserving its fallback.
static bool CoOp_InstallLifeHooks() {
    const u8 die[]={0x56,0x8B,0xF1,0xE8,0x18,0x2F,0xFA,0xFF};
    if(memcmp(reinterpret_cast<void*>(0x004BD7E0),die,8))return false;
    const u32 sites[]={0x004B28FF,0x004B5EF3,0x004BDB10,0x004E147B,0x00455BD3};
    for(unsigned i=0;i<5;++i) {
        const u8* c=reinterpret_cast<const u8*>(sites[i]);
        if(c[0]!=0xE8 || sites[i]+5+*reinterpret_cast<const i32*>(c+1)!=(i==4?0x00460700:0x004BDFF0))return false;
    }
    const u8 end[]={0xA3,0xB0,0xB1,0x5F,0};
    if(memcmp(reinterpret_cast<void*>(0x004B28AE),end,5))return false;
    DWORD old,unused;if(!VirtualProtect(reinterpret_cast<void*>(0x00401000),0x13A000,PAGE_EXECUTE_READWRITE,&old))return false;
    *reinterpret_cast<u8*>(0x004B28AE)=0xE9;
    *reinterpret_cast<u32*>(0x004B28AF)=reinterpret_cast<u32>(&CoOp_DeathEndBridge)-0x004B28B3;
    PATCH_PUSH_RET(0x004BD7E0,CoOp_PlayerDie);
    PATCH_CALL(0x004B28FF,CoOp_DeathMode);PATCH_CALL(0x004B5EF3,CoOp_DeathMode);
    PATCH_CALL(0x004E147B,CoOp_ScriptDeath);PATCH_CALL(0x004BDB10,CoOp_DeathMode);PATCH_CALL(0x00455BD3,CoOp_GameOver);
    VirtualProtect(reinterpret_cast<void*>(0x00401000),0x13A000,old,&unused);
    FlushInstructionCache(GetCurrentProcess(),0,0);gCoOpLifeInstalled=true;return true;
}
