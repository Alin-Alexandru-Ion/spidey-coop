#include <assert.h>
#include <stdio.h>
#include "coop/world/coop_sections.h"
#include "coop/interaction/coop_interactions.h"
#include "coop/script/coop_script_motion.h"
struct Item {unsigned short mFlags;Item* mNextItem;};
#include "bank_fixture.h"
static CoOpSections<Item> bank;
static Item terrain[9];
static unsigned owner,visits;
static unsigned raceVisits;
static Item sewerRooms[2];
static unsigned sewerVisits;
static void VisitSewer(const char* name,int first,int last,bool visible,void*) {
    int room=!strcmp(name,"room1_")?0:!strcmp(name,"room2_")?1:-1;
    assert(room>=0 && first==1 && last==(room==0?18:52));
    bank.Remember(&sewerRooms[room]);
    bank.Command(&sewerRooms[room],owner,visible);++sewerVisits;
}
static void VisitRace(const char* name,int first,int last,bool visible,void*) {
    assert(!strcmp(name,"Inside") && first==1 && last==1 && !visible);
    ++raceVisits;
}
static void Visit(const char* name,int first,int last,bool visible,void*) {
    const char* names[]={"Start_","First_","Second_","Third_","Fourth_","Fifth_","Sixth_","Seventh_","Eight_"};
    assert(first>=0 && last<100);
    for(int i=0;i<9;++i)if(!strcmp(names[i],name)) {
        bank.Remember(&terrain[i]);bank.Command(&terrain[i],owner,visible);++visits;return;
    }
    assert(false);
}
static unsigned TouchCommands(unsigned short* commands,unsigned N,unsigned player) {
    owner=player;return CoOp_VisitSectionCommands(commands,commands+N,Visit,0);
}
#define Touch(commands,player) TouchCommands(commands,sizeof(commands)/sizeof(commands[0]),player)
static void BankScenario(unsigned lead,unsigned trailing) {
    bank.Clear();visits=0;
    for(int i=0;i<9;++i){terrain[i].mFlags=0x820;bank.Remember(&terrain[i]);}
    assert(Touch(bank67,lead)==5); // enable middle, release Seventh for leading player
    assert(!(terrain[7].mFlags&1)); // trailing player still needs Seventh
    assert(Touch(bank70,0)==5); // unowned cinematic repeats same five commands
    assert(!(terrain[7].mFlags&1)); // scene cannot clear trailing player's hold
    assert(Touch(bank67,trailing)==5);assert(terrain[7].mFlags&1); // both now left
    // Native latch is still set by the other participant: no retail execution.
    // Contact projection must still enable the returning player's section.
    assert(Touch(bank71,trailing)==2);assert(!(terrain[7].mFlags&1));
    assert(Touch(bank68,lead)==2);assert(!(terrain[6].mFlags&1));
    assert(Touch(bank68,trailing)==2);assert(terrain[6].mFlags&1);
    assert(Touch(bank67,trailing)==5);assert(!(terrain[6].mFlags&1));
    // Every original non-visibility flag survives these actual authored commands.
    for(i=0;i<9;++i)assert((terrain[i].mFlags&~1)==0x820);
    unsigned short pulse[]={0x86,0xffff,3,0xffff};
    assert(!Touch(pulse,lead)); // never replay links or other gameplay commands
    unsigned short truncated[]={0xbf,'F'|('i'<<8)};
    assert(!Touch(truncated,lead));
    unsigned short unrelated[]={0xbf,'S'|('_'<<8),'F'|('l'<<8),'o'|('o'<<8),'r',2,2,0};
    assert(!Touch(unrelated,lead)); // puzzle floor retains native world semantics
}
struct SCommandPoint {u32 Checksum;unsigned NodeIndex;u16* pCommands;SCommandPoint* pNextSimilar;};
static SCommandPoint* hashTable[256];
static u16* nodeTable[2];static int nodeCount=2;
static CoOpSectionOwners gCoOpNodeOwners;
static CoOpDeferredOwners gCoOpDeferredOwners;
static struct {bool enabled;} gCoOpSession={true};
struct PendingListEntry {unsigned short NodeIndex;unsigned short pad;u16* pCommands;};
static PendingListEntry queue[16];
static unsigned CoOp_PendingCount(){unsigned n=0;while(n<16 && queue[n].pCommands)++n;return n;}
static bool gCoOpSectionsActive=true,interceptFall=false;
static bool gCoOpRaceSectionsActive=false;
static bool gCoOpSewerSectionsActive=false;
static CoOpSectionProfile gCoOpSectionProfile=COOP_SECTION_TERRAIN;
static bool deferRaceContact;
static bool CoOp_DeferRaceContact(unsigned node,unsigned player) {
    return deferRaceContact && node==330 && (player==1 || player==2);
}
static void SewerScenario(unsigned lead,unsigned trailing) {
    bank.Clear();sewerVisits=0;
    for(int i=0;i<2;++i){sewerRooms[i].mFlags=0x820;bank.Remember(&sewerRooms[i]);}
    owner=lead;
    assert(CoOp_VisitSectionCommands(sewer223,sewer223+
        sizeof(sewer223)/sizeof(sewer223[0]),VisitSewer,0,COOP_SECTION_SEWER)==1);
    assert(!(sewerRooms[0].mFlags&1)); // trailing player still needs room 1
    assert(CoOp_VisitSectionCommands(sewer225,sewer225+
        sizeof(sewer225)/sizeof(sewer225[0]),VisitSewer,0,COOP_SECTION_SEWER)==1);
    assert(!(sewerRooms[1].mFlags&1)); // its room 2 remains present too
    owner=trailing;
    assert(CoOp_VisitSectionCommands(sewer223,sewer223+
        sizeof(sewer223)/sizeof(sewer223[0]),VisitSewer,0,COOP_SECTION_SEWER)==1);
    assert(sewerRooms[0].mFlags&1); // only now have both players left room 1
    assert(CoOp_VisitSectionCommands(sewer224,sewer224+
        sizeof(sewer224)/sizeof(sewer224[0]),VisitSewer,0,COOP_SECTION_SEWER)==1);
    assert(!(sewerRooms[0].mFlags&1)); // returning player reacquires the floor
    assert(sewerVisits==4);
    for(int j=0;j<2;++j)assert((sewerRooms[j].mFlags&~1)==0x820);
}
static unsigned gCoOpTriggerOwner=1,gCoOpSectionTouches[2],nativeCalls;
static CoOpScriptMotionState gCoOpScriptMotion;
static unsigned gCoOpCollisionScripts[2];
static bool latched;
static struct Players {
    bool secondary;CoOpPlayerRouter router;
    bool InSecondaryContext(){return secondary;}
    const CoOpPlayerRouter& Router()const{return router;}
} gCoOpPlayers;
static bool CoOp_InterceptFallTrigger(u32,unsigned){return interceptFall;}
static SCommandPoint* __cdecl NativeTrigger(u32 checksum,bool) {
    ++nativeCalls;if(latched)return 0;latched=true;
    SCommandPoint* point=hashTable[checksum&255];
    unsigned n=CoOp_PendingCount();queue[n].NodeIndex=point->NodeIndex;queue[n].pCommands=point->pCommands;
    return point;
}
static void CoOp_SectionContact(const char* name,int first,int last,bool visible,void* context) {
    owner=*static_cast<unsigned*>(context);Visit(name,first,last,visible,0);
}
static unsigned CoOp_ProjectSectionGraph(unsigned,unsigned){assert(false);return 0;}
#include "section_trigger_test.h"
static void NativeContactScenario() {
    CPlayer p1,p2;gCoOpPlayers.router.Publish(COOP_PLAYER_ONE,&p1);
    gCoOpPlayers.router.Publish(COOP_PLAYER_TWO,&p2);
    SCommandPoint point={42,0,bank67,0};hashTable[42]=&point;
    nodeTable[0]=bank67;nodeTable[1]=bank67+sizeof(bank67)/sizeof(bank67[0]);
    bank.Clear();
    for(int i=0;i<9;++i){terrain[i].mFlags=0x821;bank.Remember(&terrain[i]);}
    latched=true;gCoOpPlayers.secondary=true;
    assert(CoOp_SectionTrigger(42,false)==0); // native still suppresses its shared script
    if(nativeCalls!=1 || gCoOpSectionTouches[1]!=5)
        fprintf(stderr,"calls=%u touches=%u enabled=%d active=%d owner=%u visits=%u nodes=%p,%p cmds=%p\n",nativeCalls,gCoOpSectionTouches[1],gCoOpSession.enabled,gCoOpSectionsActive,gCoOpNodeOwners.Get(0),visits,nodeTable[0],nodeTable[1],point.pCommands);
    assert(nativeCalls==1 && gCoOpSectionTouches[1]==5);
    assert(!(terrain[6].mFlags&1)); // actual wrapper nevertheless acquired P2's terrain
    owner=1;Visit("Sixth_",0,35,false,0);assert(!(terrain[6].mFlags&1));
    owner=2;Visit("Sixth_",0,35,false,0);assert(terrain[6].mFlags&1);
    gCoOpPlayers.secondary=false;
    assert(CoOp_SectionTrigger(42,false)==0);
    assert(gCoOpSectionTouches[0]==5 && !(terrain[6].mFlags&1));
    assert(gCoOpNodeOwners.Get(0)==3);
    unsigned contacts=gCoOpSectionTouches[0];
    gCoOpSectionsActive=false;CoOp_SectionTrigger(42,false);
    assert(gCoOpSectionTouches[0]==contacts && nativeCalls==3); // no terrain projection
    memset(queue,0,sizeof(queue));latched=false;gCoOpPlayers.secondary=true;
    assert(CoOp_SectionTrigger(42,false)==&point);
    assert(CoOp_PendingCount()==1 && gCoOpDeferredOwners.Get(bank67,0).slot==COOP_PLAYER_TWO);
    assert(gCoOpCollisionScripts[1]==1 && gCoOpCollisionScripts[0]==0);
    assert(gCoOpNodeOwners.Get(0)==3); // ownership works outside L1A3
    memset(queue,0,sizeof(queue));gCoOpDeferredOwners.Clear();gCoOpPlayers.secondary=false;
    gCoOpSectionsActive=true;interceptFall=true;CoOp_SectionTrigger(42,false);
    assert(nativeCalls==4 && gCoOpSectionTouches[0]==contacts); // fall interception retained
    interceptFall=false;
    point.NodeIndex=330;deferRaceContact=true;
    unsigned callsBefore=nativeCalls;
    assert(CoOp_SectionTrigger(42,false)==0 && nativeCalls==callsBefore);
    deferRaceContact=false;point.NodeIndex=0;
}
struct LateCase {const char* name;Item item;unsigned matches;};
static void VisitLate(const char* name,int first,int last,bool visible,void* context) {
    LateCase* test=static_cast<LateCase*>(context);
    assert(first<=last && last<100);
    if(strcmp(name,test->name))return;
    assert(bank.Remember(&test->item));
    bank.Command(&test->item,owner,visible);++test->matches;
}
static void LateScenario(CoOpSectionProfile profile,const char* name,
    unsigned short* hide,unsigned hideCount,unsigned expectedHide,
    unsigned short* show,unsigned showCount,unsigned expectedShow) {
    assert(CoOp_IsProtectedSection(name,profile));
    for(unsigned first=1;first<=2;++first) {
        bank.Clear();LateCase test={name,{0x820,0},0};
        assert(bank.Remember(&test.item));
        owner=first;
        assert(CoOp_VisitSectionCommands(hide,hide+hideCount,VisitLate,&test,profile)==expectedHide);
        assert(!(test.item.mFlags&1)); // other player still in this room
        owner=3-first;
        assert(CoOp_VisitSectionCommands(hide,hide+hideCount,VisitLate,&test,profile)==expectedHide);
        assert(test.item.mFlags&1); // both left, original area may hide
        owner=first;
        assert(CoOp_VisitSectionCommands(show,show+showCount,VisitLate,&test,profile)==expectedShow);
        assert(!(test.item.mFlags&1)); // returning player reacquires room
        assert(test.matches==3 && (test.item.mFlags&~1)==0x820);
    }
}
#define CHECK_LATE(profile,name,label,hideCount,showCount) \
    LateScenario(profile,name,late##label##Hide, \
        sizeof(late##label##Hide)/sizeof(late##label##Hide[0]),hideCount, \
        late##label##Show,sizeof(late##label##Show)/sizeof(late##label##Show[0]),showCount)
int main() {
    const unsigned short raceRelease[]={0xbf,'T'|('R'<<8),'G'|('P'<<8),'_'|('W'<<8),
        'a'|('i'<<8),'t',3,14,0,0xbf,'I'|('n'<<8),'s'|('i'<<8),
        'd'|('e'<<8),0,1,1,0,0xffff};
    assert(CoOp_VisitSectionCommands(raceRelease,raceRelease+
        sizeof(raceRelease)/sizeof(raceRelease[0]),VisitRace,0,COOP_SECTION_RACE)==1 && raceVisits==1);
    BankScenario(1,2);BankScenario(2,1);
    SewerScenario(1,2);SewerScenario(2,1);NativeContactScenario();
    CoOpSectionOwners owners;
    owners.Record(38,1);owners.Record(38,2);
    assert(owners.Get(38)==3); // same command point hit by both
    owners.Record(70,owners.Get(38));assert(owners.Get(70)==3); // linked pulse
    owners.Record(425,2);assert(owners.Get(425)==2);
    assert(owners.Get(999999)==0);
    owners.Clear();assert(!owners.Get(38) && !owners.Get(70));
    CoOpSections<Item> sections;
    Item start={0x820,0},next={0x801,0};start.mNextItem=&next;
    assert(sections.Remember(&start) && sections.Remember(&next));
    sections.Command(&start,2,false);
    assert(!(start.mFlags&1)); // stationary P1 still requires start
    sections.Command(&next,2,true);assert(!(next.mFlags&1));
    sections.Command(&start,1,false);assert(start.mFlags&1); // both left
    sections.Command(&start,2,true);assert(!(start.mFlags&1)); // P2 returns
    sections.ApplyLive(&start,1);assert(start.mFlags&1); // P2 drops out
    sections.Rejoin(&start);assert(start.mFlags&1); // rejoin uses P1 requirement
    sections.Command(&start,1,true);assert(!(start.mFlags&1));
    sections.Command(&start,1,false);assert(start.mFlags&1);
    sections.Command(&start,3,true);assert(!(start.mFlags&1)); // shared trigger
    sections.Command(&start,0,false);assert(!(start.mFlags&1)); // scene cannot evict either player
    sections.Command(&start,1,false);assert(!(start.mFlags&1));
    sections.Command(&start,2,false);assert(start.mFlags&1); // both released
    sections.Command(&start,0,true);assert(!(start.mFlags&1)); // shared display
    sections.Command(&start,0,false);assert(start.mFlags&1); // no player holds it
    assert((start.mFlags&~1)==0x820 && (next.mFlags&~1)==0x800);
    start.mNextItem=0;sections.ApplyLive(&start,1); // removed pointer never read
    sections.Clear();assert(sections.Count()==0);
    assert(CoOp_SectionProfileForResource("L7A1_G.psx")==COOP_SECTION_L7A1);
    assert(CoOp_SectionProfileForResource("l8a3_g")==COOP_SECTION_L8A3);
    assert(CoOp_SectionProfileForResource("l7a5_g")==COOP_SECTION_NONE);
    assert(CoOp_SectionProfileForResource("l9a1_g")==COOP_SECTION_NONE);
    assert(!CoOp_IsProtectedSection("TRGP_HELP",COOP_SECTION_L7A1));
    assert(!CoOp_IsProtectedSection("B_LIT",COOP_SECTION_L7A1));
    assert(!CoOp_IsProtectedSection("BARRIER",COOP_SECTION_L7A3));
    assert(!CoOp_IsProtectedSection("A_SMOKE",COOP_SECTION_L7A4));
    assert(!CoOp_IsProtectedSection("W1_Kid",COOP_SECTION_L8A1));
    assert(!CoOp_IsProtectedSection("Intro_",COOP_SECTION_L8A2));
    assert(!CoOp_IsProtectedSection("ScreenA1",COOP_SECTION_L8A3));
    CHECK_LATE(COOP_SECTION_L7A1,"B_ROOM",Office,9,9);
    CHECK_LATE(COOP_SECTION_L7A2,"A_INNER",Cables,3,8);
    CHECK_LATE(COOP_SECTION_L7A3,"A_ROOM",Press,5,4);
    CHECK_LATE(COOP_SECTION_L7A4,"A_HALL",Furnace,5,5);
    CHECK_LATE(COOP_SECTION_L8A1,"Ware1_",Warehouse,1,1);
    CHECK_LATE(COOP_SECTION_L8A2,"GearRoomA_",Gear,1,1);
    CHECK_LATE(COOP_SECTION_L8A3,"RoomA",Control,1,1);
    // Mirror: stationary P2 must survive P1 leaving.
    start.mFlags=0;sections.Remember(&start);
    sections.Command(&start,1,false);assert(!(start.mFlags&1));
    sections.Command(&start,2,false);assert(start.mFlags&1);
    puts("PASS installed early and late bank scripts: seven room/terrain profiles, both roles, departure, return, shared scenes and unrelated flags");
    return 0;
}
