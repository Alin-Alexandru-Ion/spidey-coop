// Co-op / script: Coordinate authored motion and cinematics with co-op control handoffs.

#ifndef COOP_CRANE_FALLBACK_H
#define COOP_CRANE_FALLBACK_H

// The broken crane motion falls continuously, so waiting for a stationary
// actor is unsafe. Substitute only its exact authored input command, before
// it starts. Ordinary chase openings and singleplayer keep native behavior.
inline bool CoOp_CraneAutoReady(bool coop,bool craneBank,int node,
    const unsigned short* commands,unsigned words) {
    static const unsigned short route[]={199,0,2,157,253,3,7,1500,0,255,65535};
    if(!coop || !craneBank || node!=158 || !commands || words<11)return false;
    for(unsigned i=0;i<11;++i)if(commands[i]!=route[i])return false;
    return true;
}
// Accept the known crane exit commands before bypassing the stalled route.
inline bool CoOp_CraneExitMatches(const unsigned short* commands,unsigned words) {
    // The original opcode 305, its parameters, L3A5_T string, and terminator.
    static const unsigned short exit[]={305,110,162,181,0x334C,0x3541,0x545F,0,65535};
    if(!commands || words<9)return false;
    for(unsigned i=0;i<9;++i)if(commands[i]!=exit[i])return false;
    return true;
}
// Verify the authored crane arrival route before applying the fallback.
inline bool CoOp_CraneArrivalRouteMatches(const unsigned short* commands,unsigned words) {
    // L3A5 restart node 0 pulses node 22. Its first synthesized operation is
    // CutSceneSkipSnapPos(26); node 26 also supplies the native exit facing.
    static const unsigned short arrival[]={199,0,16,26,0,2,23,72,3,7,50,0,2,26,0,255,65535};
    if(!commands || words<17)return false;
    for(unsigned i=0;i<17;++i)if(commands[i]!=arrival[i])return false;
    return true;
}
#endif
