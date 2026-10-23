#include <assert.h>
#include <stdio.h>
#include "router_test_types.h"
#include "coop/script/coop_script_motion.h"

int main() {
    CoOpScriptMotionState state;
    CVector start={0,0,0},next={4096,0,0};
    CoOpPlayerHandle p1(COOP_PLAYER_ONE,1),p2(COOP_PLAYER_TWO,2);
    assert(!state.active && !state.Partner(COOP_PLAYER_ONE));
    assert(!state.Begin(CoOpPlayerHandle(),(void*)1,start,0));
    assert(state.Begin(p2,(void*)1,start,(void*)5));
    assert(state.Owns(p2) && state.Partner(COOP_PLAYER_ONE));
    assert(!state.Partner(COOP_PLAYER_TWO));
    assert(!state.Begin(p1,(void*)1,start,0));
    state.Observe(true,(void*)5,start);
    assert(state.motionTicks==1 && state.stalledTicks==1 && !state.waitingForPartner);
    state.Observe(true,(void*)6,next);
    assert(state.motionTicks==2 && state.stalledTicks==0);
    state.Observe(false,(void*)6,next);
    assert(state.waitingForPartner && state.last.vx==4096);
    state.Clear();
    assert(!state.active && state.Begin(p1,(void*)2,start,0));
    assert(state.Partner(COOP_PLAYER_TWO));
    puts("PASS: scripted motion retains the real actor until native control release");
    return 0;
}
