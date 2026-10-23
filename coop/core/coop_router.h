// Co-op / core: Own the two-player roster and scoped compatibility state for retail globals.
#pragma once
#ifndef COOP_ROUTER_H
#define COOP_ROUTER_H
#include "spidey.h"

enum CoOpPlayerSlot { COOP_PLAYER_ONE = 0, COOP_PLAYER_TWO = 1, COOP_PLAYER_COUNT = 2 };
struct CoOpPlayerHandle
{
    u32 slot;
    unsigned __int64 generation;
    // Initialize co-op player handle with no active retail ownership.
    CoOpPlayerHandle() : slot(COOP_PLAYER_COUNT), generation(0) {}
    // Initialize co-op player handle with no active retail ownership.
    CoOpPlayerHandle(u32 s, unsigned __int64 g) : slot(s), generation(g) {}
};
enum CoOpOperation
{
    COOP_RENDER, COOP_POSE, COOP_CAMERA, COOP_MOTION,
    COOP_INPUT, COOP_DAMAGE,
    COOP_ENEMY_TARGET, COOP_INTERACTION, COOP_PICKUP
};

// Non-owning routing table. Registry alone publishes/removes participants.
// Returned pointers are for immediate use; queued work must store handles.
class CoOpPlayerRouter
{
public:
    struct Participant
    {
        CPlayer* player; // Every participant is a real CPlayer
        unsigned __int64 generation;
    };
    typedef bool (*Condition)(CoOpPlayerHandle, const CPlayer*, void*);
    struct Query
    {
        CoOpOperation operation;
        CVector origin;
        double maxDistance; // world units; -1 means unlimited, full 3D distance
        CoOpPlayerHandle preferred; // retain only while still eligible
        Condition condition; // caller supplies LOS, trigger bounds, action state
        void* context;
        // Update query in the authoritative two-player roster.
        Query() : operation(COOP_ENEMY_TARGET), maxDistance(-1),
            // Update condition in the authoritative two-player roster.
            condition(0), context(0) {}
    };
    // Initialize co-op player router with no active retail ownership.
    CoOpPlayerRouter()
    {
        for (u32 i = 0; i < COOP_PLAYER_COUNT; ++i)
        {
            mSlots[i].player = 0;
            mSlots[i].generation = 0;
        }
    }
    // Update publish in the authoritative two-player roster.
    void Publish(CoOpPlayerSlot slot, CPlayer* player)
    {
        if ((u32)slot >= COOP_PLAYER_COUNT || !player) return;
        Participant& p = mSlots[slot];
        if (p.player == player) return;
        ++p.generation;
        if (!p.generation) ++p.generation;
        p.player = player;
    }
    // Remove withdraw from the active roster and scoped retail globals.
    void Withdraw(CoOpPlayerSlot slot)
    {
        if ((u32)slot >= COOP_PLAYER_COUNT) return;
        // Preserve generation across clears; next publication is a new lifetime.
        mSlots[slot].player = 0;
    }
    // Release owned resources and reset the active roster and scoped retail globals to its inactive state.
    void Clear()
    {
        for (u32 i = 0; i < COOP_PLAYER_COUNT; ++i)
            Withdraw(static_cast<CoOpPlayerSlot>(i));
    }
    // Update handle in the authoritative two-player roster.
    CoOpPlayerHandle Handle(CoOpPlayerSlot slot) const
    {
        if ((u32)slot >= COOP_PLAYER_COUNT || !mSlots[slot].player)
            return CoOpPlayerHandle();
        return CoOpPlayerHandle(slot, mSlots[slot].generation);
    }
    const Participant* Resolve(CoOpPlayerHandle h) const
    {
        if (h.slot >= COOP_PLAYER_COUNT || !h.generation) return 0;
        const Participant& p = mSlots[h.slot];
        return p.player && p.generation == h.generation ? &p : 0;
    }
    // Input/hit/camera/pose ownership never falls back to another participant.
    CoOpPlayerHandle Owned(CoOpOperation operation, CoOpPlayerHandle owner) const
    {
        if (operation != COOP_RENDER && operation != COOP_POSE &&
            operation != COOP_CAMERA && operation != COOP_MOTION && operation != COOP_INPUT &&
            operation != COOP_DAMAGE && operation != COOP_PICKUP) return CoOpPlayerHandle();
        return Eligible(operation, owner) ? owner : CoOpPlayerHandle();
    }
    // Update select in the authoritative two-player roster.
    CoOpPlayerHandle Select(const Query& q) const
    {
        // Shared mission events and confirmed hits are NOT nearest-player queries.
        if (q.operation != COOP_ENEMY_TARGET && q.operation != COOP_INTERACTION)
            return CoOpPlayerHandle();
        CoOpPlayerHandle best;
        double bestDistance = 0;
        for (u32 i = 0; i < COOP_PLAYER_COUNT; ++i)
        {
            CoOpPlayerHandle h = Handle(static_cast<CoOpPlayerSlot>(i));
            if (!Eligible(q.operation, h)) continue;
            const Participant* p = Resolve(h);
            // Convert before subtraction/multiplication: fixed-point i32 positions
            // can overflow integer distance calculations at opposite world limits.
            double x = ((double)p->player->mPos.vx - q.origin.vx) / 4096.0;
            double y = ((double)p->player->mPos.vy - q.origin.vy) / 4096.0;
            double z = ((double)p->player->mPos.vz - q.origin.vz) / 4096.0;
            double distance = x*x + y*y + z*z;
            if (q.maxDistance >= 0 && distance > q.maxDistance*q.maxDistance) continue;
            if (q.condition && !q.condition(h, p->player, q.context)) continue;
            if (h.slot == q.preferred.slot && h.generation == q.preferred.generation)
                return h;
            if (best.slot == COOP_PLAYER_COUNT || distance < bestDistance)
            {
                best = h;
                bestDistance = distance;
            }
        }
        return best; // invalid means no eligible player; caller decides defer/cancel
    }
private:
    // Update eligible in the authoritative two-player roster.
    bool Eligible(CoOpOperation operation, CoOpPlayerHandle h) const
    {
        const Participant* p = Resolve(h);
        if (!p) return false;
        if (operation == COOP_RENDER || operation == COOP_POSE || operation == COOP_CAMERA || operation == COOP_MOTION)
            return true;
        // Health is a baseline filter, not a replacement for IsDead/invulnerability
        // or mission-specific conditions. Retail adapters must add those checks.
        return p->player && p->player->mHealth > 0;
    }
    CoOpPlayerRouter(const CoOpPlayerRouter&);
    CoOpPlayerRouter& operator=(const CoOpPlayerRouter&);
    Participant mSlots[COOP_PLAYER_COUNT];
};
#endif
