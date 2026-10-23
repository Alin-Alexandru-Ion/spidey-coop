#include <assert.h>
#include <stdio.h>
#include "coop/script/coop_crane_fallback.h"
#include "crane_fixture.h"
int main() {
    unsigned short* route=craneBankWords+craneRouteCommandWord;
    unsigned short* exit=craneBankWords+craneExitCommandWord;
    assert(CoOp_CraneAutoReady(true,true,158,route,11));
    assert(CoOp_CraneExitMatches(exit,9));
    assert(CoOp_CraneArrivalRouteMatches(arrivalBankWords+arrivalCommandWord,17));
    assert(!CoOp_CraneArrivalRouteMatches(arrivalBankWords+arrivalCommandWord,16));
    assert(!CoOp_CraneAutoReady(false,true,158,route,11));
    assert(!CoOp_CraneAutoReady(true,false,158,route,11));
    assert(!CoOp_CraneAutoReady(true,true,70,route,11));
    assert(!CoOp_CraneAutoReady(true,true,158,route,10));
    assert(!CoOp_CraneExitMatches(exit,8));
    unsigned short original=route[3];route[3]=127;
    assert(!CoOp_CraneAutoReady(true,true,158,route,11));route[3]=original;
    original=exit[4];exit[4]=0;
    assert(!CoOp_CraneExitMatches(exit,9));exit[4]=original;
    puts("PASS installed crane route and authored exit signatures; singleplayer, unrelated route, truncated/changed commands rejected");
    return 0;
}
