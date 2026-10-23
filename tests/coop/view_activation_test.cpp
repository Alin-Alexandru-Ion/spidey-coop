#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "coop/world/coop_sections.h"
#include "coop/camera/coop_view.h"
typedef unsigned char u8;
struct CItem {unsigned short mFlags;CItem* mNextItem;};
struct CPlayer {};
enum {COOP_PLAYER_TWO=1};
struct Handle {unsigned __int64 generation;};
struct Registry {
    CPlayer player;bool joined;unsigned __int64 generation;
    CPlayer* GetRenderBody(int) {return joined?&player:0;}
    Registry& Router() {return *this;}
    struct Handle Handle(int) {struct Handle h;h.generation=generation;return h;}
} gCoOpPlayers;
static u8 resources[40*0x44];
static CoOpSections<CItem> gCoOpSections;
static CoOpSectionOwners gCoOpNodeOwners;
static bool gCoOpSectionsActive;
static bool gCoOpRaceSectionsActive;
static bool gCoOpSewerSectionsActive;
static CoOpSectionProfile gCoOpSectionProfile;
static unsigned __int64 gCoOpSectionGeneration;
static const void* gCoOpSectionResource;
static CoOpSplitLayout gCoOpLayout=COOP_SPLIT_LEFT_RIGHT;
static CItem start={0,0},forward={1,0};
static CItem* CoOp_SectionLive() {return &start;}
#include "runtime_section_sync.h"
int main() {
    memcpy(resources,"l1a3_g",7);resources[10]=1;
    *reinterpret_cast<void**>(resources+0x18)=resources;
    start.mNextItem=&forward;gCoOpPlayers.joined=true;gCoOpPlayers.generation=1;
    CoOp_SyncSections();assert(gCoOpSectionsActive);
    gCoOpSections.Remember(&start);gCoOpSections.Remember(&forward);
    gCoOpSections.Command(&start,1,false); // P2 remains at start
    gCoOpSections.Command(&forward,1,true); // P1 advances
    assert(!(start.mFlags&1) && !(forward.mFlags&1));
    for(int i=0;i<16;++i) {
        gCoOpLayout=CoOp_NextView(gCoOpLayout);CoOp_SyncSections();
        assert(gCoOpSectionsActive && gCoOpSections.Count()==2);
        assert(!(start.mFlags&1) && !(forward.mFlags&1));
    }
    gCoOpPlayers.joined=false;CoOp_SyncSections();
    assert(!gCoOpSectionsActive && (start.mFlags&1) && !(forward.mFlags&1));
    memcpy(resources,"l5a1_g",7);gCoOpPlayers.joined=true;
    CoOp_SyncSections();assert(gCoOpSectionsActive && gCoOpRaceSectionsActive);
    assert(CoOp_IsRaceSection("Inside") && CoOp_IsRaceSection("Outside") &&
        !CoOp_IsRaceSection("TRGP_Wait"));
    gCoOpSections.Remember(&start);gCoOpSections.Command(&start,2,true);
    gCoOpSections.Command(&start,1,false);
    assert(!(start.mFlags&1)); // P2's interior remains present when P1 exits
    memcpy(resources,"l5a3_g",7);
    static char sewerResource;
    *reinterpret_cast<void**>(resources+0x18)=&sewerResource;
    CoOp_SyncSections();assert(gCoOpSectionsActive && gCoOpSewerSectionsActive &&
        !gCoOpRaceSectionsActive);
    assert(CoOp_IsSewerSection("room1_") && CoOp_IsSewerSection("room2_") &&
        !CoOp_IsSewerSection("green_light_"));
    start.mFlags=0;gCoOpSections.Remember(&start);
    gCoOpSections.Command(&start,2,false); // P2 advances out of the first room
    assert(!(start.mFlags&1)); // stationary P1 still owns its floor
    gCoOpSections.Command(&start,1,false);assert(start.mFlags&1);
    gCoOpSections.Command(&start,2,true);assert(!(start.mFlags&1));
    memcpy(resources,"l7a1_g",7);
    static char officeResource;
    *reinterpret_cast<void**>(resources+0x18)=&officeResource;
    CoOp_SyncSections();assert(gCoOpSectionsActive && gCoOpSectionProfile==COOP_SECTION_L7A1);
    assert(CoOp_IsProtectedSection("B_ROOM",gCoOpSectionProfile));
    assert(!CoOp_IsProtectedSection("B_LIT",gCoOpSectionProfile));
    gCoOpSections.Remember(&start);assert(gCoOpSections.Count()==1);
    memcpy(resources,"l8a3_g",7);
    // Retail may recycle the same PSX address for a new bank.
    CoOp_SyncSections();assert(gCoOpSectionsActive && gCoOpSectionProfile==COOP_SECTION_L8A3);
    assert(gCoOpSections.Count()==0);
    assert(CoOp_IsProtectedSection("RoomA",gCoOpSectionProfile));
    assert(!CoOp_IsProtectedSection("ScreenA1",gCoOpSectionProfile));
    memcpy(resources,"l7a5_g",7); // boss stage changes remain shared
    CoOp_SyncSections();assert(!gCoOpSectionsActive && gCoOpSectionProfile==COOP_SECTION_NONE);
    puts("PASS actual runtime section sync: 16 view switches retain P2 floor and P1 zone; actual departure releases only P2 zone");
    return 0;
}
