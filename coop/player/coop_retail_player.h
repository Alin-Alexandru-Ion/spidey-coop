// Co-op / player: Construct, update, carry, and recover each independent retail CPlayer.
#pragma once
#ifndef COOP_RETAIL_PLAYER_H
#define COOP_RETAIL_PLAYER_H
#include "spidey.h"
#include "camera.h"
#include "../core/coop_context.h"
#include "../input/coop_input.h"
#include "coop_spawn.h"
#include "coop_costume.h"
#include "../core/coop_session.h"
#include "coop_entry_state.h"

// Original CPlayer and CCamera, independently constructed. The authoritative
// roster lives in the registry. MechList/CameraList become scoped compatibility
// contexts for legacy routines, not a replacement for the two-player roster.
class CoOpRetailPlayer
{
public:
    // Initialize co-op retail player with no active retail ownership.
    CoOpRetailPlayer() : player(0),camera(0),ticks(0),failureReason(0),spawnStrategy(0) {
        CoOp_ClearControl(control);memset(view,0,sizeof(view));
    }
    // Return the current value owned by this module without changing retail state.
    CPlayer* Get() const {return player;}
    // Report whether this instance currently owns its native context.
    bool Active() const {return bank.Active();}
    // Return how many native update ticks P2 has completed.
    unsigned long Ticks() const {return ticks;}
    // Return the last P2 creation failure code for diagnostics.
    unsigned FailureReason() const {return failureReason;}
    // Return the placement route chosen for P2's current spawn.
    unsigned SpawnStrategy() const {return spawnStrategy;}
    // Return this participant's independent CCamera.
    CCamera* Camera() const {return camera;}
    // Copy the latest native camera frame only when both body and camera exist.
    bool CopyView(void* out) const {
        if(!player || !camera) return false;
        memcpy(out,view,sizeof(view));return true;
    }
    // Construct a separate retail CPlayer and camera with scoped globals and validated resources.
    bool Create(CPlayer* primary,bool shadowStart=false,const CoOpPlayerCarry* carry=0) {
        failureReason=0;
        if(!primary || !*reinterpret_cast<CCamera**>(0x0056F3B8)) {failureReason=1;return false;}
        const u8* region=reinterpret_cast<const u8*>(0x006B2440)+primary->mRegion*0x44;
        if(!region[9] || !region[10] ||
            !*reinterpret_cast<const u32*>(region+0x18)) {failureReason=2;return false;}
        CVector spawn;
        bool chaseBank=false;
        for(int r=0;r<40;++r) {
            const u8* entry=reinterpret_cast<const u8*>(0x006B2440)+r*0x44;
            if(entry[10] && !_strnicmp((const char*)entry,"L3A",3)) {
                chaseBank=true;break;
            }
        }
        u8 spawnGte[0x100];memcpy(spawnGte,reinterpret_cast<void*>(0x00610B20),sizeof(spawnGte));
        // At a level's authored opening, construct P2 even when P1 is in a
        // wall, swing, fall or synthesized route. P2 shadows that position
        // invisibly until the native control handoff supplies a safe release.
        bool safeSpawn=shadowStart || CoOp_FindSpawn(primary,spawn,chaseBank);
        unsigned chosenStrategy=shadowStart?3:gCoOpLastSpawnStrategy;
        if(shadowStart)spawn=primary->mPos;
        memcpy(reinterpret_cast<void*>(0x00610B20),spawnGte,sizeof(spawnGte));
        if(!safeSpawn) {failureReason=3;return false;}
        Destroy();
        if(!PrepareBank()) {bank.Clear();failureReason=4;return false;}
        CoOp_ClearControl(control);ticks=0;
        // SetSpideyCamValue's authored presets, not P1's live interpolation.
        const u32 presetAddresses[]={0x6A81DC,0x6A81DE,0x6A81E0,0x6A81FA,0x6A8274,
            0x6A81E4,0x6A81E2,0x6A81E6,0x6A81F8,0x6A8C66,
            0x6A8204,0x6A8200,0x6A8202,0x6A825C,0x6A8CAC,
            0x6A8C5E,0x6A8C56,0x6A8C54,0x6A8C68,0x6A8196,
            0x6A8194,0x6A8192,0x6A8198,0x6A82A0,0x6A8190};
        short presets[25];
        for(unsigned n=0;n<25;++n) presets[n]=*reinterpret_cast<short*>(presetAddresses[n]);
        {
            CoOpContextScope scope(bank);
            if(!scope.Entered()) {failureReason=5;return false;}
            // Use the same costume IDs/unlocks as the retail viewer. Continue
            // may load a different save after the co-op submenu was visited,
            // so validate again against the active slot at construction time.
            unsigned suit=CoOp_CostumeValidated(gCoOpSelectedSuit,CoOp_CostumeUnlockMask());
            *reinterpret_cast<i32*>(0x005559DC)=suit;
            *reinterpret_cast<i32*>(0x006B78F8)=0;
            typedef i32 (__cdecl *SpoolFn)(const char*,i32);
            i32 suitRegion=reinterpret_cast<SpoolFn>(0x004CAC20)(gCoOpSuitResources[suit],0);
            if(suitRegion<0 || suitRegion>=40){failureReason=6;return false;}
            const u8* suitEntry=reinterpret_cast<const u8*>(0x006B2440)+suitRegion*0x44;
            if(!suitEntry[9] || !suitEntry[10] || !*reinterpret_cast<const u32*>(suitEntry+0x18)){failureReason=7;return false;}
            *reinterpret_cast<CPlayer**>(0x006A9038)=0;
            *reinterpret_cast<u32*>(0x006A9034)=0;
            if(carry && carry->valid) {
                *reinterpret_cast<i32*>(0x006828A0)=carry->webbing;
                *reinterpret_cast<i32*>(0x006828A4)=carry->cartridges;
                *reinterpret_cast<i32*>(0x006828A8)=carry->armorHealth;
            }
            *reinterpret_cast<CCamera**>(0x0056F3B8)=0;
            *reinterpret_cast<u32*>(0x0056F3B4)=0;
            // The player AI/destructor traverses this owned effect list.
            *reinterpret_cast<void**>(0x006B55A0)=0;
            memset(reinterpret_cast<void*>(0x0060F750),0,0x24);
            typedef void* (__cdecl *AllocFn)(u32);
            typedef CPlayer* (FASTCALL *CtorFn)(CPlayer*,int);
            void* memory=reinterpret_cast<AllocFn>(0x00455390)(sizeof(CPlayer));
            if(memory) {
                memset(memory,0,sizeof(CPlayer));
                // Skip the legacy shared texture swap; the suit is a separate resource.
                *reinterpret_cast<u8*>(0x006A9040)=1;
                *reinterpret_cast<u8*>(0x006828D1)=0;
                player=reinterpret_cast<CtorFn>(0x004B9EB0)((CPlayer*)memory,0);
                // 006A9040=1 is only a constructor guard against shared suit
                // mutations. Do not retain a false "already armored" state:
                // retail SetArmor(true) otherwise returns before setting 5E9.
                *reinterpret_cast<u8*>(0x006A9040)=player->field_5E9?1:0;
                // The constructor resets these globals. A late join inherits
                // the level's tuned presets while keeping independent motion.
                for(unsigned n=0;n<25;++n)
                    *reinterpret_cast<short*>(presetAddresses[n])=presets[n];
                if(carry && carry->valid) {
                    reinterpret_cast<void (FASTCALL*)(CPlayer*,int,bool)>(0x004BAEC0)(player,0,carry->armor);
                    carry->Apply(player);
                }
                player->field_E0C=reinterpret_cast<i32>(&control);
                // The chase bank's native restart can begin on an attached
                // surface or in midair. Construct at P1's exact authored
                // coordinate before applying a verified sideways separation.
                player->mPos=primary->mPos;
                if(chaseBank || shadowStart)player->mVel=primary->mVel;
                player->mPos=spawn;
                player->mAngles=primary->mAngles;
                player->mTransform=primary->mTransform;
                player->mpLight=primary->mpLight;player->mRGB=primary->mRGB;
                player->mCBodyFlags&=~3;
                player->field_7C=*reinterpret_cast<i32*>(0x006B4CA8);
                typedef CCamera* (FASTCALL *CamCtorFn)(CCamera*,int,CBody*);
                memory=reinterpret_cast<AllocFn>(0x00455390)(sizeof(CCamera));
                if(memory) {
                    memset(memory,0,sizeof(CCamera));
                    camera=reinterpret_cast<CamCtorFn>(0x00415EE0)((CCamera*)memory,0,player);
                    typedef void (FASTCALL *Fn)(void*,int);
                    reinterpret_cast<Fn>(0x00416AA0)(camera,0);
                    reinterpret_cast<Fn>(0x00416A20)(camera,0);
                    memcpy(view,reinterpret_cast<void*>(0x0056F1B0),sizeof(view));
                }
            }
        }
        if(!player || !camera) {Destroy();failureReason=8;return false;}
        spawnStrategy=chosenStrategy;
        OutputDebugStringA("Co-op: real CPlayer and CCamera constructed for P2.\n");
        return true;
    }
    // Update shadow on the selected CPlayer's independent lifecycle.
    void Shadow(CPlayer* primary) {
        if(!player || !primary)return;
        CoOpContextScope scope(bank);
        if(scope.Entered())CoOp_ShadowEntry(player,primary);
    }
    // Update release from actor on the selected CPlayer's independent lifecycle.
    bool ReleaseFromActor(CPlayer* actor,const CVector& position,CCamera* actorCamera) {
        if(!player || !camera || !actor)return false;
        CoOpContextScope scope(bank);
        if(!scope.Entered())return false;
        u8 gte[0x100];memcpy(gte,reinterpret_cast<void*>(0x00610B20),sizeof(gte));
        bool adopted=CoOp_AdoptEntryMotion(player,actor,position);
        if(adopted)CoOp_AdoptEntryCamera(camera,actorCamera);
        memcpy(reinterpret_cast<void*>(0x00610B20),gte,sizeof(gte));
        if(adopted)spawnStrategy=gCoOpLastSpawnStrategy;
        return adopted;
    }
    // Update release shadow on the selected CPlayer's independent lifecycle.
    bool ReleaseShadow(const CVector& position,short yaw) {
        if(!player || !camera)return false;
        CoOpContextScope scope(bank);
        if(!scope.Entered())return false;
        player->mPos=position;
        player->mAngles.vx=player->mAngles.vz=0;player->mAngles.vy=yaw;
        player->mVel.vx=player->mVel.vy=player->mVel.vz=0;
        player->mAcc.vx=player->mAcc.vy=player->mAcc.vz=0;
        short angles[3]={0,yaw,0};
        reinterpret_cast<void (FASTCALL*)(CPlayer*,void*,short*)>(0x004B9E50)(player,0,angles);
        reinterpret_cast<void (FASTCALL*)(CPlayer*,void*)>(0x004BE4B0)(player,0);
        reinterpret_cast<void (FASTCALL*)(CCamera*,void*)>(0x00416AA0)(camera,0);
        spawnStrategy=1;
        return true;
    }
    // Update with context on the selected CPlayer's independent lifecycle.
    bool WithContext(void (*callback)(void*),void* argument) {
        if(!player || !camera || !callback) return false;
        CoOpContextScope scope(bank);
        if(!scope.Entered()) return false;
        callback(argument);
        return true;
    }
    // Update tick on the selected CPlayer's independent lifecycle.
    void Tick(const CoOpInput& input) {
        if(!player || !camera) return;
        CoOpContextScope scope(bank);
        if(!scope.Entered()) return;
        if(spawnStrategy==3 && ticks<180 && ticks%15==0) {
            // An airborne join may initially overlap P1. Once this same
            // body's nearby collision becomes available, separate sideways.
            u8 probeGte[0x100];
            memcpy(probeGte,reinterpret_cast<void*>(0x00610B20),sizeof(probeGte));
            CVector side;
            if(CoOp_FindSpawn(player,side,true) && gCoOpLastSpawnStrategy==2) {
                player->mPos=side;spawnStrategy=2;
            }
            memcpy(reinterpret_cast<void*>(0x00610B20),probeGte,sizeof(probeGte));
        }
        typedef void (__cdecl *ButtonFn)(SButton*,int);
        CoOp_FillControl(control,input,reinterpret_cast<ButtonFn>(0x00479540));
        // Original camera code also reads global controls directly.
        SControl* global=reinterpret_cast<SControl*>(0x00661100);
        *global=control;
        global->pTriangle=&global->LeftOne;global->pSquare=&global->LeftTwo;
        global->pCircle=&global->RightOne;global->pX=&global->RightTwo;
        player->field_E0C=reinterpret_cast<i32>(&control);
        typedef void (FASTCALL *Fn)(void*,int);
        // Player updates can leave surface/camera math in the emulated GTE.
        // Restore the enclosing P1 call's registers when returning into its AI.
        u8 gte[0x100];memcpy(gte,reinterpret_cast<void*>(0x00610B20),sizeof(gte));
        reinterpret_cast<Fn>(0x00460F90)(player,0);
        memcpy(reinterpret_cast<void*>(0x00610B20),gte,sizeof(gte));
        ++ticks;
    }
    // Advance update camera using the current per-player state.
    void UpdateCamera() {
        if(!player || !camera) return;
        CoOpContextScope scope(bank);
        if(!scope.Entered()) return;
        typedef void (FASTCALL *Fn)(void*,int);
        u8 gte[0x100];memcpy(gte,reinterpret_cast<void*>(0x00610B20),sizeof(gte));
        reinterpret_cast<Fn>(0x00460F90)(camera,0);
        reinterpret_cast<Fn>(0x00416A20)(camera,0);
        memcpy(view,reinterpret_cast<void*>(0x0056F1B0),sizeof(view));
        memcpy(reinterpret_cast<void*>(0x00610B20),gte,sizeof(gte));
    }
    // Release owned resources and reset the selected CPlayer without overwriting the other player's state to its inactive state.
    void Destroy() {
        if(player || camera) {
            CoOpContextScope scope(bank);
            if(scope.Entered()) {
                // Exclude shared checkpoint and costume teardown. Context
                // restoration does not undo frees, so owned lists are isolated.
                *reinterpret_cast<i32*>(0x0060CFA4)=1;
                *reinterpret_cast<u8*>(0x006A9040)=0;
                *reinterpret_cast<u8*>(0x006A9041)=0;
                typedef void* (FASTCALL *DeleteFn)(void*,int,u32);
                if(player) reinterpret_cast<DeleteFn>(0x004C9210)(player,0,1);
                player=0;
                if(camera) {
                    DeleteFn destroy=reinterpret_cast<DeleteFn>((*reinterpret_cast<u32**>(camera))[0]);
                    destroy(camera,0,1);
                }
                camera=0;
            }
        }
        bank.Clear();ticks=0;failureReason=0;spawnStrategy=0;
    }
private:
    CPlayer* player;
    CCamera* camera;
    SControl control;
    u8 view[0x94];
    CoOpContextBank bank;
    unsigned long ticks;
    unsigned failureReason;
    unsigned spawnStrategy;
    // Prepare prepare bank for the selected CPlayer without overwriting the other player's state.
    bool PrepareBank() {
        bank.Clear();
        // Deliberately exclude shared environment/actors, world clock and queues.
        const u32 spans[][2]={
            {0x006A7FE0,0x1054}, // player animation/action tables, camera tuning
            {0x006A9034,8},     // context player count and MechList
            {0x006A9040,2},     // costume flags
            {0x005559DC,4},    // per-player CurrentSuit (Ben Reilly = 8)
            {0x006B78F8,4},    // high-detail suit lookup in the P2 context
            {0x006A906C,0x1C},  // player target scratch
            {0x00548858,0x14},  // original camera distance/offset values
            {0x00556498,4},     // animation timing selector
            {0x005559F4,0x20},  // animation timing constants
            {0x0056EFF0,0x3FC}, // camera scratch, MikeCamera, count, CameraList
            {0x00661100,0x18C}, // direct global input readers
            {0x0066129C,4},     // Pad_IdleTime
            {0x006B55A0,4},     // player-owned effect list
            {0x0060F750,0x24},  // private HUD portraits and panel offsets; base sprites supplied at draw
            {0x006828A0,0xC},{0x006828D1,2}, // checkpoint inventory
            {0x0060CFBC,4},{0x0060CFC4,1}, // player death/controller flags
            {0x0060CFA4,4},     // lifecycle mode
            {0x00660F80,4},     // letterbox flag cleared by destructor
            {0x006A7484,0x14}, // target position, radius and spin belong to the aiming player
            {0x006A7572,1}      // destructor's 48AA40 flag
        };
        for(u32 i=0;i<sizeof(spans)/sizeof(spans[0]);++i)
            if(!bank.Add(reinterpret_cast<void*>(spans[i][0]),spans[i][1])) return false;
        // Constructor clears high bits through these command pointers. Preserve
        // the pointees too; just copying their pointers would still corrupt P1.
        for(u32 a=0x006A82B8;a<0x006A8768;a+=4) {
            u32* words=*reinterpret_cast<u32**>(a);
            if(!words) continue;
            u32 n=0;
            for(;n<4096;++n) {
                if(IsBadReadPtr(words+n,4)) return false;
                if(words[n]==0xFFFFFFFF) break;
            }
            if(n==4096 || !bank.Add(words,(n+1)*4)) return false;
        }
        if(!bank.Add(reinterpret_cast<void*>(0x005565A8),0x68)) return false;
        return bank.Capture();
    }
    CoOpRetailPlayer(const CoOpRetailPlayer&);
    CoOpRetailPlayer& operator=(const CoOpRetailPlayer&);
};
#endif
