// Co-op / world: Keep collision, objects, and section activation valid around both players.
#pragma once
#ifndef COOP_SECTIONS_H
#define COOP_SECTIONS_H
#include <string.h>
class CoOpSectionOwners {
    unsigned char owners[65536];
public:
    // Initialize co-op section owners with no active retail ownership.
    CoOpSectionOwners() {Clear();}
    // Release owned resources and reset the union of both players' active regions to its inactive state.
    void Clear() {memset(owners,0,sizeof(owners));}
    // Track record while retaining geometry needed by either player.
    void Record(unsigned node,unsigned mask) {if(node<65536) owners[node]|=(unsigned char)(mask&3);}
    // Return the current value owned by this module without changing retail state.
    unsigned Get(unsigned node) const {return node<65536?owners[node]:0;}
};
// Only the leading, authored terrain visibility commands are projected to
// each collision participant. Never replay pulses, actors, rewards or scenes.
inline bool CoOp_IsTerrainSection(const char* prefix) {
    static const char* names[]={"Start_","First_","Second_","Third_","Fourth_",
        "Fifth_","Sixth_","Seventh_","Eight_","BUILD","GIRD","RBOX"};
    for(unsigned i=0;i<sizeof(names)/sizeof(names[0]);++i)
        if(!strcmp(prefix,names[i]))return true;
    return false;
}
// Recognize the Venom race's inside and outside visibility groups.
inline bool CoOp_IsRaceSection(const char* prefix) {
    return !strcmp(prefix,"Inside") || !strcmp(prefix,"Outside");
}
// Recognize sewer rooms, halls and tanks that can unload independently.
inline bool CoOp_IsSewerSection(const char* prefix) {
    return !strcmp(prefix,"room1_") || !strcmp(prefix,"room2_") ||
        !strcmp(prefix,"hallway1_") || !strcmp(prefix,"hallway2_") || !strcmp(prefix,"tanks1_");
}
enum CoOpSectionProfile {
    COOP_SECTION_NONE,COOP_SECTION_TERRAIN,COOP_SECTION_RACE,COOP_SECTION_SEWER,
    COOP_SECTION_L7A1,COOP_SECTION_L7A2,COOP_SECTION_L7A3,COOP_SECTION_L7A4,
    COOP_SECTION_L8A1,COOP_SECTION_L8A2,COOP_SECTION_L8A3
};
// The resource identifies which authored visibility names mean persistent
// scenery. Puzzle switches, boss phases, difficulty variants and effects keep
// their shared retail behavior. All accepted names were read from installed
// trigger banks; the same two-player ownership logic serves every bank.
inline CoOpSectionProfile CoOp_SectionProfileForResource(const char* name) {
    if(!name)return COOP_SECTION_NONE;
    static const char* banks[]={"l1a3_g","l5a1_g","l5a3_g","l5a6_g","l3a3_g",
        "l7a1_g","l7a2_g","l7a3_g","l7a4_g","l8a1_g","l8a2_g","l8a3_g"};
    static const CoOpSectionProfile profiles[]={COOP_SECTION_TERRAIN,COOP_SECTION_RACE,
        COOP_SECTION_SEWER,COOP_SECTION_SEWER,COOP_SECTION_TERRAIN,
        COOP_SECTION_L7A1,COOP_SECTION_L7A2,COOP_SECTION_L7A3,COOP_SECTION_L7A4,
        COOP_SECTION_L8A1,COOP_SECTION_L8A2,COOP_SECTION_L8A3};
    for(unsigned i=0;i<sizeof(banks)/sizeof(banks[0]);++i)
        if(!_strnicmp(name,banks[i],6) && (!name[6] || name[6]=='.'))return profiles[i];
    return COOP_SECTION_NONE;
}
// Keep authored geometry loaded while either player still owns its section.
inline bool CoOp_IsProtectedSection(const char* prefix,CoOpSectionProfile profile) {
    if(!prefix)return false;
    if(profile==COOP_SECTION_TERRAIN)return CoOp_IsTerrainSection(prefix);
    if(profile==COOP_SECTION_RACE)return CoOp_IsRaceSection(prefix);
    if(profile==COOP_SECTION_SEWER)return CoOp_IsSewerSection(prefix);
    static const char* l7a1[]={"AB_CEIL","AB_ROOM","AC_","A_CEIL","A_DESK","A_FILE",
        "A_ROOM","A_SERV","A_STUFF","A_VENT","BC_CEIL","BC_ROOM","B_CEIL",
        "B_COMP_","B_FILE","B_LIGHT","B_OUT","B_ROOM","B_SERV","B_TERM",
        "B_TSHAD","B_VENT","C_CABLE","C_CEIL","C_FILE","C_OUT","C_ROOM","C_SIGN","X_VENT"};
    static const char* l7a2[]={"A2_CABLE","A_CABLE","A_INNER","A_OUTER",
        "B2_CABLE","B_CABLE","B_INNER","B_OUTER","C2_CABLE","C_CABLE",
        "C_INNER","C_OUTER","D2_CABLE","D_CABLE","D_INNER","D_OUTER","CORR_"};
    static const char* l7a3[]={"AB_ROOM","A_LIGHT","A_PAPER","A_PRESS","A_ROOM",
        "BC_ROOM","BD_ROOM","B_LIGHT","B_PRESS","B_ROOM","C_LIGHT","C_ROOM",
        "D_LIGHT","D_PAPER","D_ROOM","X_PAPER","X_ROOM","Y_LIGHT","Y_ROOM"};
    static const char* l7a4[]={"A_FURN","A_HALL","A_LIGHT","A_OVEN","A_PIPES",
        "B_FURN","B_HALL","B_LIGHT","B_OVEN","B_PIPE","C_HALL","C_LIGHT",
        "D_HALL","D_LIGHT","D_OVEN","E_HALL","E_LIGHT","F_HALL","F_LIGHT",
        "R_ROOM","T_ROOM"};
    static const char* l8a1[]={"Under_","Ware1_","Ware2_","WareA_","WareTransA","WareTransB"};
    static const char* l8a2[]={"GearRoomA_","GearRoomB_","TrenchA_","TrenchB_","TrenchC_"};
    static const char* l8a3[]={"MainA","MainB","MainC","MainD","RoomA","RoomB","RoomC","RoomS"};
    const char* const* names=0;unsigned count=0;
    switch(profile) {
    case COOP_SECTION_L7A1:names=l7a1;count=sizeof(l7a1)/sizeof(l7a1[0]);break;
    case COOP_SECTION_L7A2:names=l7a2;count=sizeof(l7a2)/sizeof(l7a2[0]);break;
    case COOP_SECTION_L7A3:names=l7a3;count=sizeof(l7a3)/sizeof(l7a3[0]);break;
    case COOP_SECTION_L7A4:names=l7a4;count=sizeof(l7a4)/sizeof(l7a4[0]);break;
    case COOP_SECTION_L8A1:names=l8a1;count=sizeof(l8a1)/sizeof(l8a1[0]);break;
    case COOP_SECTION_L8A2:names=l8a2;count=sizeof(l8a2)/sizeof(l8a2[0]);break;
    case COOP_SECTION_L8A3:names=l8a3;count=sizeof(l8a3)/sizeof(l8a3[0]);break;
    default:return false;
    }
    for(unsigned i=0;i<count;++i)if(!strcmp(prefix,names[i]))return true;
    return false;
}
typedef void (*CoOpSectionVisit)(const char*,int,int,bool,void*);
// Walk only leading visibility opcodes; leave gameplay scripts untouched.
inline unsigned CoOp_VisitSectionCommands(const unsigned short* commands,
    const unsigned short* end,CoOpSectionVisit visit,void* context,
    CoOpSectionProfile profile=COOP_SECTION_TERRAIN) {
    unsigned count=0;
    while(commands<end && *commands==0xbf && count<16) {
        const char* name=reinterpret_cast<const char*>(++commands);
        const char* limit=reinterpret_cast<const char*>(end);
        unsigned n=0;while(name+n<limit && n<13 && name[n])++n;
        if(name+n>=limit || n==13)break;
        commands+=((n+1)+1)/2; // retail strings are padded to a word boundary
        if(end-commands<3 || commands[0]>commands[1] || commands[1]>=100 || commands[2]>1)break;
        // L5A1's first visibility command targets TRGP_Wait; its Inside
        // command follows immediately. Keep scanning the leading visibility
        // run, but project only the groups whose ownership we manage.
        if(CoOp_IsProtectedSection(name,profile)) {
            visit(name,commands[0],commands[1],commands[2]!=0,context);
            ++count;
        }
        commands+=3;
    }
    return count;
}
// The two participants keep independent authored section visibility requirements.
// Store no owning references. Apply/leave only while traversing the live list.
template<class Item,unsigned Capacity=1024> class CoOpSections {
    struct Entry {Item* item;unsigned visible;};
    Entry entries[Capacity];
    unsigned count;
public:
    // Initialize co-op sections with no active retail ownership.
    CoOpSections():count(0) {}
    // Release owned resources and reset the union of both players' active regions to its inactive state.
    void Clear() {count=0;}
    // Track remember while retaining geometry needed by either player.
    bool Remember(Item* item) {
        for(unsigned i=0;i<count;++i) if(entries[i].item==item) return true;
        if(count==Capacity) return false;
        entries[count].item=item;
        entries[count++].visible=(item->mFlags&1)?0:3;
        return true;
    }
    // Track command while retaining geometry needed by either player.
    void Command(Item* item,unsigned owners,bool visible) {
        for(unsigned i=0;i<count;++i) if(entries[i].item==item) {
            // A shared scene may display scenery, but cannot erase a live
            // participant's requirement for terrain in its own section.
            if(!owners) {Apply(item,visible?3:entries[i].visible);return;}
            if(visible) entries[i].visible|=owners;
            else entries[i].visible&=~owners;
            Apply(item,entries[i].visible);
            return;
        }
    }
    // Track rejoin while retaining geometry needed by either player.
    void Rejoin(Item* live) {
        for(unsigned i=0;i<count;++i)
            entries[i].visible=(entries[i].visible&1)?3:0;
        ApplyLive(live,3);
    }
    // Apply live to the union of both players' active regions.
    void ApplyLive(Item* live,unsigned participants) {
        for(Item* item=live;item;item=static_cast<Item*>(item->mNextItem))
            for(unsigned i=0;i<count;++i) if(entries[i].item==item) {
                Apply(item,entries[i].visible&participants);break;
            }
    }
    // Return count from the union of both players' active regions.
    unsigned Count() const {return count;}
private:
    // Track apply while retaining geometry needed by either player.
    static void Apply(Item* item,unsigned visible) {
        if(visible) item->mFlags&=~1;
        else item->mFlags|=1;
    }
};
#endif
