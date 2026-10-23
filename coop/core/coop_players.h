// Co-op / core: Own the two-player roster and scoped compatibility state for retail globals.
#pragma once
#ifndef COOP_PLAYERS_H
#define COOP_PLAYERS_H
#include "spidey.h"
#include "coop_router.h"
#include "../player/coop_retail_player.h"
static const u32 COOP_RETAIL_MECHLIST_ADDRESS=0x006A9038;

// Both slots are CPlayers. MechList is a legacy execution context; the router
// owns the authoritative roster. No independently allocated CSuper preview.
class CoOpPlayerRegistry
{
public:
    // Initialize co-op player registry with no active retail ownership.
    CoOpPlayerRegistry() : primary(0),carry(0),spawnRequested(false),removeRequested(false),primaryReplaced(false),shadowSecondary(false),spawnAttempts(0),spawnFailures(0) {}
    // Release owned resources and reset the active roster and scoped retail globals to its inactive state.
    void Clear() {
        router.Clear();secondary.Destroy();primary=0;
        spawnRequested=removeRequested=primaryReplaced=shadowSecondary=false;spawnGate.Reset();
        spawnAttempts=spawnFailures=0;
    }
    // Report whether P2's bank currently owns the retail globals.
    bool InSecondaryContext() const {return secondary.Active();}
    // Refresh P1's registry entry after the game replaces its CPlayer.
    bool SyncPrimaryFromRetail() {
        if(InSecondaryContext()) return primary!=0;
        CPlayer* current=*reinterpret_cast<CPlayer**>(COOP_RETAIL_MECHLIST_ADDRESS);
        if(current!=primary) {
            const bool replaced=primary!=0 || primaryReplaced;
            Clear();primary=current;
            // The retail player can be replaced before the render hook sees
            // the new level. Latch that transition across other sync calls.
            primaryReplaced=replaced;
        }
        if(primary) router.Publish(COOP_PLAYER_ONE,primary);
        return primary!=0;
    }
    // Update router in the authoritative two-player roster.
    const CoOpPlayerRouter& Router() const {return router;}
    // Update consume primary replacement in the authoritative two-player roster.
    bool ConsumePrimaryReplacement() {
        bool replaced=primaryReplaced;primaryReplaced=false;return replaced;
    }
    // Keep an authored interaction on its surviving player owner.
    CoOpPlayerHandle RouteOwned(CoOpOperation op,CoOpPlayerHandle owner) {
        SyncPrimaryFromRetail();return router.Owned(op,owner);
    }
    // Choose the eligible player for a requested interaction.
    CoOpPlayerHandle Route(const CoOpPlayerRouter::Query& query) {
        SyncPrimaryFromRetail();return router.Select(query);
    }
    // Return the current value owned by this module without changing retail state.
    CPlayer* Get(CoOpPlayerSlot slot) const {
        const CoOpPlayerRouter::Participant* p=router.Resolve(router.Handle(slot));
        return p ? p->player : 0;
    }
    // Return the player body that should be drawn in this viewport.
    CPlayer* GetRenderBody(CoOpPlayerSlot slot) const {return Get(slot);}
    // Queue P2 construction until the next safe game tick.
    void RequestSecondarySpawn() {if(SyncPrimaryFromRetail()) {
        // The co-op submenu arms a new participant for each level lifetime.
        // A repeated render/menu request must never reconstruct a live one.
        if(Get(COOP_PLAYER_TWO))return;
        if(!spawnRequested) spawnGate.Reset();spawnRequested=true;
    }}
    // Queue P2 destruction outside the current retail callback.
    void RequestSecondaryRemoval() {
        if(Get(COOP_PLAYER_TWO) && (primary->mHealth<=0 || Get(COOP_PLAYER_TWO)->mHealth<=0)) return;
        spawnRequested=false;spawnGate.Reset();removeRequested=true;router.Withdraw(COOP_PLAYER_TWO);
    }
    // Update secondary camera in the authoritative two-player roster.
    CCamera* SecondaryCamera() const {return secondary.Camera();}
    // Store set carry in the active roster and scoped retail globals.
    void SetCarry(const CoOpPlayerCarry* state) {carry=state;}
    // Snapshot P2's health, webbing and armor for the next level load.
    void CaptureCarry(CoOpPlayerCarry& state,int nativeExitMode) const {
        state.Capture(Get(COOP_PLAYER_TWO));
    }
    // Report whether P2 is invisible during an authored handoff.
    bool SecondaryShadowed() const {return shadowSecondary;}
    // Advance update secondary camera using the current per-player state.
    void UpdateSecondaryCamera() {if(Get(COOP_PLAYER_TWO)) secondary.UpdateCamera();}
    // Return copy secondary camera from the active roster and scoped retail globals.
    bool CopySecondaryCamera(void* out) const {return Get(COOP_PLAYER_TWO) && secondary.CopyView(out);}
    // Update place secondary in the authoritative two-player roster.
    bool PlaceSecondary(const CVector& position,short yaw) {
        if(!Get(COOP_PLAYER_TWO) || !secondary.ReleaseShadow(position,yaw))return false;
        shadowSecondary=false;return true;
    }
    // Release P2 from its entry shadow when retail control resumes.
    bool HandoffSecondary(CPlayer* actor,const CVector& position,CCamera* actorCamera) {
        if(!Get(COOP_PLAYER_TWO) || !secondary.ReleaseFromActor(actor,position,actorCamera))return false;
        shadowSecondary=false;return true;
    }
    // Update shadow secondary in the authoritative two-player roster.
    void ShadowSecondary(CPlayer* actor) {
        if(Get(COOP_PLAYER_TWO) && actor)secondary.Shadow(actor);
    }
    // Update secondary ticks in the authoritative two-player roster.
    unsigned long SecondaryTicks() const {return secondary.Ticks();}
    // Prepare spawn requested for the active roster and scoped retail globals.
    bool SpawnRequested() const {return spawnRequested;}
    // Prepare spawn attempts for the active roster and scoped retail globals.
    unsigned SpawnAttempts() const {return spawnAttempts;}
    // Prepare spawn failures for the active roster and scoped retail globals.
    unsigned SpawnFailures() const {return spawnFailures;}
    // Prepare spawn failure reason for the active roster and scoped retail globals.
    unsigned SpawnFailureReason() const {return secondary.FailureReason();}
    // Return the placement route chosen for P2's current spawn.
    unsigned SpawnStrategy() const {return secondary.SpawnStrategy();}
    // Advance tick secondary using the current per-player state.
    void TickSecondary(const CoOpInput& input) {
        CPlayer* p=Get(COOP_PLAYER_TWO);
        if(!p) return;
        if(shadowSecondary) {secondary.Shadow(primary);return;}
        // Retain the participant and its camera at zero HP. The co-op life
        // layer owns waiting/revival; zero health is never a manual drop-out.
        if(p->mHealth<=0) return;
        secondary.Tick(input);
    }
    // Run a retail callback with P2's bank installed, then restore P1's globals.
    bool WithSecondaryContext(void (*callback)(void*),void* argument) {
        return Get(COOP_PLAYER_TWO) && secondary.WithContext(callback,argument);
    }
    // Draw draw secondary only in the owning player's viewport.
    void DrawSecondaryOnly(bool hidden=false) {
        CPlayer* p=Get(COOP_PLAYER_TWO);
        if(p && !shadowSecondary && !hidden) {
            typedef void (__cdecl *RenderFn)(void*);
            reinterpret_cast<RenderFn>(0x004739A0)(p);
        }
    }
    // Draw render secondary in the owning player's viewport.
    void RenderSecondary(bool hidden=false) {
        if(!SyncPrimaryFromRetail()) return;
        if(removeRequested) {secondary.Destroy();shadowSecondary=false;removeRequested=false;}
        // Model regions can differ (P2 is Ben Reilly). Player/level lifetime is
        // owned by SyncPrimaryFromRetail and Clear, not by costume equality.
        if(spawnRequested && primary->mHealth>0 && !CoOp_SpawnBlocked(true) &&
           (!spawnFailures || spawnGate.Ready(false,GetTickCount()))) {
            ++spawnAttempts;
            bool created=secondary.Create(primary,true,carry);
            if(!created)++spawnFailures;
            if(created)shadowSecondary=true;
            if(created) router.Withdraw(COOP_PLAYER_TWO); // new lifetime even if allocator reused the address
            if(secondary.Get()) router.Publish(COOP_PLAYER_TWO,secondary.Get());
            else router.Withdraw(COOP_PLAYER_TWO);
            // Failed support/clearance leaves an existing partner intact and
            // keeps a pending join. Retry at most once per settling interval.
            spawnRequested=!created;spawnGate.Reset();
        }
        if(shadowSecondary && !hidden && !CoOp_SpawnBlocked()) {
            u8 gte[0x100];memcpy(gte,reinterpret_cast<void*>(0x00610B20),sizeof(gte));
            CVector side;bool safe=CoOp_FindEntrySpawn(primary,side);
            memcpy(reinterpret_cast<void*>(0x00610B20),gte,sizeof(gte));
            CCamera* actorCamera=*reinterpret_cast<CCamera**>(0x0056F3B8);
            if(safe && secondary.ReleaseFromActor(primary,side,actorCamera))
                shadowSecondary=false;
        }
        if(shadowSecondary)secondary.Shadow(primary);
        DrawSecondaryOnly(hidden);
    }
private:
    CoOpPlayerRouter router;
    CPlayer* primary;
    CoOpRetailPlayer secondary;
    const CoOpPlayerCarry* carry;
    bool spawnRequested,removeRequested,primaryReplaced,shadowSecondary;
    unsigned spawnAttempts,spawnFailures;
    CoOpSpawnGate spawnGate;
};
#endif
