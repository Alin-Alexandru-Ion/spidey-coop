// Co-op / script: Coordinate authored motion and cinematics with co-op control handoffs.
#pragma once
#ifndef COOP_SCRIPT_MOTION_H
#define COOP_SCRIPT_MOTION_H
#include "../core/coop_router.h"

// An authored input route is a single shared-world event executed by its real
// triggering CPlayer. Keep that owner through later script pulses and body
// ticks; the trigger dispatch alone is not the whole scripted movement.
struct CoOpScriptMotionState {
    CoOpPlayerHandle owner;
    const void* bank;
    CVector entry;
    CVector last;
    unsigned long ticks;
    unsigned long motionTicks;
    unsigned long stalledTicks;
    const void* cursor;
    bool active;
    bool sawNativeMotion;
    bool waitingForPartner;
    // Initialize co-op script motion state with no active retail ownership.
    CoOpScriptMotionState() {Clear();}
    // Release owned resources and reset the authored scene and its recorded player owner to its inactive state.
    void Clear() {
        owner=CoOpPlayerHandle();bank=0;
        entry.vx=entry.vy=entry.vz=last.vx=last.vy=last.vz=0;
        ticks=motionTicks=stalledTicks=0;cursor=0;
        active=sawNativeMotion=waitingForPartner=false;
    }
    // Track begin until the authored control handoff completes.
    bool Begin(CoOpPlayerHandle actor,const void* scriptBank,const CVector& position,
        const void* routeCursor) {
        if(actor.slot>=COOP_PLAYER_COUNT || !actor.generation || !scriptBank)return false;
        if(active && (owner.slot!=actor.slot || owner.generation!=actor.generation))return false;
        if(active) {
            if(waitingForPartner) {
                entry=last=position;cursor=routeCursor;
                ticks=motionTicks=stalledTicks=0;waitingForPartner=false;
            }
            return true;
        }
        owner=actor;bank=scriptBank;entry=last=position;cursor=routeCursor;
        active=sawNativeMotion=true;return true;
    }
    // Track owns until the authored control handoff completes.
    bool Owns(CoOpPlayerHandle h) const {
        return active && owner.slot==h.slot && owner.generation==h.generation;
    }
    // Track partner until the authored control handoff completes.
    bool Partner(unsigned participant) const {
        return active && participant<COOP_PLAYER_COUNT && owner.slot!=participant;
    }
    // Track observe until the authored control handoff completes.
    void Observe(bool nativeMotion,const void* routeCursor,const CVector& position) {
        if(!active)return;
        ++ticks;
        if(nativeMotion) {
            sawNativeMotion=true;waitingForPartner=false;++motionTicks;
            if(cursor==routeCursor && last.vx==position.vx &&
                last.vy==position.vy && last.vz==position.vz)++stalledTicks;
            else stalledTicks=0;
        }
        cursor=routeCursor;last=position;
        if(sawNativeMotion && !nativeMotion)waitingForPartner=true;
    }
};
#endif
