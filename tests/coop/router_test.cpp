#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "coop/core/coop_router.h"
static bool OnlyP2(CoOpPlayerHandle h, const CPlayer*, void*)
{ return h.slot == COOP_PLAYER_TWO; }
int main()
{
    CPlayer a, b;
    memset(&a,0,sizeof(a)); memset(&b,0,sizeof(b));
    a.mHealth=b.mHealth=100;
    a.mPos.vx=10*4096; b.mPos.vx=2*4096;
    CoOpPlayerRouter router; CoOpPlayerRouter::Query q;
    q.origin.vx=q.origin.vy=q.origin.vz=0;
    assert(!router.Resolve(router.Select(q)));
    router.Publish(COOP_PLAYER_ONE,&a);
    CoOpPlayerHandle h1=router.Handle(COOP_PLAYER_ONE);
    router.Publish(COOP_PLAYER_TWO,0);
    assert(!router.Resolve(router.Handle(COOP_PLAYER_TWO)));
    router.Publish(COOP_PLAYER_TWO,&b);
    CoOpPlayerHandle h2=router.Handle(COOP_PLAYER_TWO);
    assert(router.Select(q).slot==COOP_PLAYER_TWO);
    q.preferred=h1; assert(router.Select(q).slot==COOP_PLAYER_ONE);
    q.maxDistance=5; assert(router.Select(q).slot==COOP_PLAYER_TWO);
    q.maxDistance=2; assert(router.Select(q).slot==COOP_PLAYER_TWO);
    q.maxDistance=1; assert(!router.Resolve(router.Select(q)));
    q.maxDistance=-1; q.condition=OnlyP2;
    assert(router.Select(q).slot==COOP_PLAYER_TWO);
    b.mHealth=0; assert(!router.Resolve(router.Select(q)));
    q.condition=0; assert(router.Select(q).slot==COOP_PLAYER_ONE);
    assert(!router.Resolve(router.Owned(COOP_DAMAGE,h2))); // no victim fallback
    b.mHealth=100; q.preferred=CoOpPlayerHandle();
    q.operation=COOP_DAMAGE; assert(!router.Resolve(router.Select(q)));
    q.operation=COOP_INTERACTION; assert(router.Select(q).slot==COOP_PLAYER_TWO);
    router.Withdraw(COOP_PLAYER_TWO);
    assert(!router.Resolve(router.Owned(COOP_INPUT,h2)));
    router.Publish(COOP_PLAYER_TWO,&b); // same address, new lifetime
    assert(!router.Resolve(h2));
    h2=router.Handle(COOP_PLAYER_TWO);
    router.Publish(COOP_PLAYER_TWO,&b); assert(router.Resolve(h2));
    q.operation=COOP_ENEMY_TARGET; q.preferred=CoOpPlayerHandle();
    a.mPos.vx=b.mPos.vx=4096;
    assert(router.Select(q).slot==COOP_PLAYER_ONE); // stable tie
    a.mPos.vx=0x7fffffff; b.mPos.vx=(-2147483647-1);
    q.origin.vx=(-2147483647-1);
    assert(router.Select(q).slot==COOP_PLAYER_TWO);
    assert(!router.Resolve(router.Handle(static_cast<CoOpPlayerSlot>(99))));
    router.Clear(); router.Publish(COOP_PLAYER_ONE,&a);
    assert(!router.Resolve(h1)); assert(!router.Resolve(h2));
    router.Publish(COOP_PLAYER_TWO,0); // null participant rejected
    assert(!router.Resolve(router.Handle(COOP_PLAYER_TWO)));
    puts("PASS: two real participants, owner routing, nearest/range/conditions, health, target retention, ties, overflow, stale handles");
    return 0;
}
