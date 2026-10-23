#include <assert.h>
#include <stdio.h>
#include "coop/world/coop_activation.h"
struct Vector { int x; };
int Distance(Vector* a,Vector* b) {int d=a->x-b->x;return d<0?-d:d;}
struct Item { unsigned short mFlags; Item* mNextItem; };
int main()
{
    Vector p1={0},p2={100000},near1={50},near2={99950},middle={50000};
    assert(CoOp_NearestActivationDistance(&p1,&near1,&p2,Distance)==50);
    assert(CoOp_NearestActivationDistance(&p1,&near2,&p2,Distance)==50);
    assert(CoOp_NearestActivationDistance(&p1,&middle,&p2,Distance)==50000);
    assert(CoOp_NearestActivationDistance(&p1,&near2,(Vector*)0,Distance)==99950);
    p1.x=-1000000; // no maximum separation limit tied to P1
    assert(CoOp_NearestActivationDistance(&p1,&near2,&p2,Distance)==50);
    for(int a=0;a<2;++a) for(int b=0;b<2;++b)
    {
        Item item={(unsigned short)(2|(a?0x8000:0)),0};
        { CoOpVisibilityUnion<Item> view(&item);
          assert(!(item.mFlags&0x8000));
          item.mFlags=(unsigned short)(0x402|(b?0x8000:0)); }
        assert((item.mFlags&0x8000)==((a&&b)?0x8000:0));
        assert((item.mFlags&0x7fff)==0x402);
    }
    {CoOpVisibilityUnion<Item> empty(0);}
    puts("PASS separate activation regions, empty gap, departure, extreme separation; visibility union preserves other flags");
    return 0;
}
