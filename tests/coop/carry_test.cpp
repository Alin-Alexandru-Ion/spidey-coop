#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "router_test_types.h"
#include "coop/core/coop_session.h"
int main() {
    CPlayer p[2];memset(p,0,sizeof(p));
    p[0].mMaxHealth=p[1].mMaxHealth=100;
    p[0].mHealth=63;p[0].mWebbing=0;p[0].field_5D8=0;
    p[0].field_5E9=true;p[0].field_5EC=17;
    p[1].mHealth=41;p[1].mWebbing=933;p[1].field_5D8=3;
    CoOpSession session;session.Select(true);
    session.primaryCarry.Capture(&p[0]);session.carry.Capture(&p[1]);session.LevelEnded();
    CPlayer next[2];memset(next,0,sizeof(next));
    session.primaryCarry.Apply(&next[0]);session.carry.Apply(&next[1]);
    assert(next[0].mHealth==63 && next[0].mWebbing==0 && next[0].field_5D8==0);
    assert(next[0].field_5E9 && next[0].field_5EC==17);
    assert(next[1].mHealth==41 && next[1].mWebbing==933 && next[1].field_5D8==3 && !next[1].field_5E9);
    for(unsigned dead=0;dead<2;++dead) {
        next[dead]=p[dead];next[dead].mHealth=0;
        CoOpPlayerCarry fresh;fresh.Capture(&next[dead]);fresh.Apply(&next[dead]);
        assert(next[dead].mHealth==100 && next[dead].mWebbing==4096 && next[dead].field_5D8==7 && !next[dead].field_5E9 && !next[dead].field_5EC);
        assert(p[1-dead].mHealth==(dead?63:41));
    }
    session.Select(false);assert(!session.primaryCarry.valid && !session.carry.valid);
    puts("PASS independent HP, armor and armor capacity, depleted supplies, level/retry carryover and death-only reset for either slot");
    return 0;
}
