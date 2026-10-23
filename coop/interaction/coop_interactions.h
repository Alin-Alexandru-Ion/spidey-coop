// Co-op / interaction: Adapt native enemy, projectile, boss, and object interactions.
#pragma once
#ifndef COOP_INTERACTIONS_H
#define COOP_INTERACTIONS_H
#include "../core/coop_router.h"
// Identity is attached to an already-confirmed contact, never selected by distance.
inline CoOpPlayerHandle CoOp_Collector(const CoOpPlayerRouter& router,const CPlayer* player) {
    for(unsigned i=0;i<COOP_PLAYER_COUNT;++i) {
        CoOpPlayerHandle h=router.Handle(static_cast<CoOpPlayerSlot>(i));
        const CoOpPlayerRouter::Participant* p=router.Resolve(h);
        if(p && p->player==player) return router.Owned(COOP_PICKUP,h);
    }
    return CoOpPlayerHandle();
}
// Retail pending list has at most 16 entries. Key includes script address so a
// different script with the same node cannot inherit a previous pickup owner.
class CoOpDeferredOwners {
    struct Entry {const void* script;int node;CoOpPlayerHandle owner;};
    Entry entries[16];unsigned count;
public:
    // Initialize co-op deferred owners with no active retail ownership.
    CoOpDeferredOwners():count(0) {}
    // Release owned resources and reset the interacting player while retaining native enemy logic to its inactive state.
    void Clear() {count=0;}
    // Route record through the interacting player's native context.
    bool Record(const void* script,int node,CoOpPlayerHandle owner) {
        if(!script || owner.slot>=COOP_PLAYER_COUNT) return false;
        for(unsigned i=0;i<count;++i) if(entries[i].script==script && entries[i].node==node)
            return true; // first confirmed collector owns this queued action
        if(count==16) return false;
        entries[count].script=script;entries[count].node=node;entries[count++].owner=owner;return true;
    }
    // Return the current value owned by this module without changing retail state.
    CoOpPlayerHandle Get(const void* script,int node) const {
        for(unsigned i=0;i<count;++i) if(entries[i].script==script && entries[i].node==node) return entries[i].owner;
        return CoOpPlayerHandle();
    }
};
// Return select enemy from the interacting player while retaining native enemy logic.
inline CoOpPlayerHandle CoOp_SelectEnemy(const CoOpPlayerRouter& router,const CVector& origin,
    CoOpPlayerRouter::Condition visible,CoOpPlayerRouter::Condition alive,void* context) {
    CoOpPlayerRouter::Query q;q.origin=origin;q.condition=visible;q.context=context;
    CoOpPlayerHandle selected=router.Select(q);
    if(!router.Resolve(selected)) {q.condition=alive;selected=router.Select(q);}
    return selected; // native AI still decides sight cone, range and attack state
}
#endif
