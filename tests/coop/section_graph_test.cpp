#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "coop/world/coop_sections.h"
#include "coop/world/coop_fall_triggers.h"
typedef unsigned short u16;
struct SCommandPoint {u16* pCommands;unsigned NodeIndex,Checksum;SCommandPoint *pNext,*pNextSimilar;};
static int nodeCount;
static u16* nodes[1024];static SCommandPoint points[1024],*pointList,*hashTable[256];
static bool gCoOpRaceSectionsActive,gCoOpSewerSectionsActive;
static CoOpSectionProfile gCoOpSectionProfile=COOP_SECTION_SEWER;
static unsigned visits,ownerMask;
static char lastPrefix[24];
struct Item {unsigned mFlags;Item* mNextItem;};
static Item hallway={0,0};static CoOpSections<Item> requirements;
static void CoOp_SectionContact(const char* name,int first,int last,bool visible,void* context) {
    ++visits;ownerMask=*static_cast<unsigned*>(context);strcpy(lastPrefix,name);
    assert(first<=last);
    if(!strcmp(name,"hallway1_"))requirements.Command(&hallway,ownerMask,visible);
}
#include "section_graph_fixture.h"
#include "section_graph_runtime_test.h"
int main() {
    LoadPlantFallFixture();gCoOpSewerSectionsActive=true;requirements.Remember(&hallway);
    unsigned result=CoOp_ProjectSectionGraph(280,1);
    if(result!=1)fprintf(stderr,"graph result=%u root=%u commands=%u,%u,%u,%u links=%u,%u childcmd=%u enddiff=%ld\n",result,nodes[280][0],points[280].pCommands[0],points[280].pCommands[1],points[280].pCommands[2],points[280].pCommands[3],nodes[280][1],nodes[280][2],points[285].pCommands[0],nodes[286]-points[285].pCommands);
    assert(result==1 && !strcmp(lastPrefix,"hallway1_") && ownerMask==1);
    assert(!(hallway.mFlags&1)); // P2 stayed in that hallway
    assert(CoOp_ProjectSectionGraph(280,2)==1 && (hallway.mFlags&1));
    assert(CoOp_ProjectSectionGraph(281,2)==1 && !(hallway.mFlags&1));
    assert(CoOp_ProjectSectionGraph(282,1)==1 && !strcmp(lastPrefix,"hallway2_"));
    assert(CoOp_ProjectSectionGraph(288,2)==1 && !strcmp(lastPrefix,"tanks1_"));
    LoadCraneFallFixture();gCoOpSewerSectionsActive=false;gCoOpSectionProfile=COOP_SECTION_TERRAIN;
    assert(CoOp_ProjectSectionGraph(280,1)==3 && !strcmp(lastPrefix,"RBOX"));
    assert(CoOp_ProjectSectionGraph(281,2)==3 && ownerMask==2);
    puts("PASS installed Sewage Plant pulse graphs: private hallway/tank ownership and reentry; crane cabin exterior groups projected to each collider without replaying box/gameplay scripts");
    return 0;
}
