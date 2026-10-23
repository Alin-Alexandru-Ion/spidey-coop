// Co-op / script: Coordinate authored motion and cinematics with co-op control handoffs.
#pragma once
#ifndef COOP_CINEMATIC_TICK_H
#define COOP_CINEMATIC_TICK_H

// Retail still updates P1's player body during an authored cinematic. A P2
// cinematic must likewise advance P2's scripted movement, while ignoring the
// player's controls. A real pause or loss of focus stops both participants.
inline bool CoOp_ShouldTickSecondary(bool paused,bool foreground,bool letterbox,
    bool secondaryOwnsCinematic) {
    return !paused && foreground && (!letterbox || secondaryOwnsCinematic);
}
// Track interactive scene owner until the authored control handoff completes.
inline bool CoOp_InteractiveSceneOwner(bool letterbox,bool sameBank,
    int primaryMode,int secondaryMode) {
    return letterbox && sameBank && primaryMode==3 &&
        (secondaryMode==4 || secondaryMode==5 || secondaryMode==6);
}
// Keep a cinematic only while its owning player remains alive and available.
inline bool CoOp_CinematicOwnerActive(bool registered,int health,bool waiting) {
    return registered && health>0 && !waiting;
}

#endif
