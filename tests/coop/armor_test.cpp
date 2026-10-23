#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "coop/core/coop_router.h"
static CPlayer p1,p2;static CPlayer* mech=&p1;
static i32 lowGraphics,armorRegion=7;
static struct {bool enabled;} gCoOpSession={true};
static struct Registry {
    bool second;
    CPlayer* Get(CoOpPlayerSlot slot) {return slot==COOP_PLAYER_ONE?&p1:(second?&p2:0);}
} gCoOpPlayers;
#include "armor_runtime_test.h"
int main() {
    memset(&p1,0,sizeof(p1));memset(&p2,0,sizeof(p2));
    p1.mRegion=2;p2.mRegion=13;gCoOpPlayers.second=true;
    for(unsigned camera=0;camera<2;++camera) {
        mech=camera?&p2:&p1;
        assert(CoOp_ArmorRenderRegion(&p1)==2 && CoOp_ArmorRenderRegion(&p2)==13);
        p1.field_5E9=true;
        assert(CoOp_ArmorRenderRegion(&p1)==7 && CoOp_ArmorRenderRegion(&p2)==13);
        p2.field_5E9=true;
        assert(CoOp_ArmorRenderRegion(&p1)==7 && CoOp_ArmorRenderRegion(&p2)==7);
        p1.field_5E9=false;
        assert(CoOp_ArmorRenderRegion(&p1)==2 && CoOp_ArmorRenderRegion(&p2)==7);
        p2.field_5E9=false;
    }
    assert(p1.mRegion==2 && p2.mRegion==13); // no shared costume/model mutation
    CSuper prop;memset(&prop,0,sizeof(prop));prop.mRegion=9;
    assert(CoOp_ArmorRenderRegion(&prop)==9);
    armorRegion=-1;p1.field_5E9=true;assert(CoOp_ArmorRenderRegion(&p1)==2);armorRegion=7;
    gCoOpSession.enabled=false;mech=&p1;
    assert(CoOp_ArmorRenderRegion(&p1)==7);
    lowGraphics=1;assert(CoOp_ArmorRenderRegion(&p1)==2);
    lowGraphics=0;gCoOpPlayers.second=false;gCoOpSession.enabled=true;
    assert(CoOp_ArmorRenderRegion(&p1)==7);
    puts("PASS armor model selection: either body in either view, independent pickup/removal, untouched costume and singleplayer behavior");
    return 0;
}
