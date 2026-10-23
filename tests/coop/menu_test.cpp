#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
typedef unsigned long u32;typedef unsigned char u8;typedef unsigned short u16;
typedef int CoOpSplitLayout;
static int gCoOpLayout;
static struct Config {int layout;} gCoOpConfig={2};
static void CoOp_LoadConfig() {}
static int costumeSaves;
static bool CoOp_SaveConfig() {++costumeSaves;return true;}
static u8 saveAvailable=1,code[0xA20];
static u32 costumeUnlocks=0x83;
static void* Address(u32 a) {if(a==0x006828D0)return &saveAvailable;if(a==0x006828D8)return &costumeUnlocks;assert(a>=0x493860 && a<0x494280);return code+a-0x493860;}
static u32 held,edges;
static u8 __cdecl Trigger(u32 mask,int edge,int) {return ((edge?edges:held)&mask)!=0;}
static void __cdecl Sound(int,int,int) {}
static void __cdecl Text(int,int,const char*,int,int) {}
static void __cdecl Cursor() {}
static int __cdecl NativeMain(int);
#include "menu_runtime_test.h"
static int replies[8],replyCount,replyIndex;
static int pages[8];static int previousValues[8];
static int __cdecl NativeMain(int previous) {
    assert(replyIndex<replyCount);
    assert(gCoOpMainSubmenu==pages[replyIndex]);
    assert(previous==previousValues[replyIndex]);
    assert(gCoOpMainCount==(gCoOpMainSubmenu==2?4:gCoOpMainSubmenu?3:8));
    assert(gCoOpMainEntries[0].action==(gCoOpMainSubmenu?2:102));
    assert(gCoOpMainEntries[1].action==(gCoOpMainSubmenu?1:100));
    if(gCoOpMainSubmenu==2) {
        assert(gCoOpMainEntries[2].action==103);
        assert(gCoOpMainEntries[3].action==101);
        assert(strstr(gCoOpMainEntries[2].label,"P2 COSTUME: ")==gCoOpMainEntries[2].label);
    }
    assert(*static_cast<u8*>(Address(0x493D08))==(gCoOpMainSubmenu?0x75:0xEB));
    assert(*static_cast<u8*>(Address(0x493D73))==gCoOpMainCount);
    // The native shell frees its preview objects before returning an action.
    gCoOpSession.LevelEnded();
    return replies[replyIndex++];
}
static void Sequence(int a,int b,int c,int d) {
    replies[0]=a;replies[1]=b;replies[2]=c;replies[3]=d;replyIndex=0;
}
enum {COOP_PLAYER_ONE=0,COOP_PLAYER_TWO=1};
static struct Registry {
    bool primary,joined,canCreate,replaced;int requests;
    bool SyncPrimaryFromRetail(){return primary;}
    bool ConsumePrimaryReplacement(){bool was=replaced;replaced=false;return was;}
    void RequestSecondarySpawn(){++requests;}
    void SetCarry(const CoOpPlayerCarry*){}
    void RenderSecondary(){if(primary && requests && canCreate)joined=true;}
    bool Get(int){return joined;}
} gCoOpPlayers;
static void __cdecl Render(void*) {}
static void CoOp_SyncSections() {}
static void CoOp_LifeResumePrimary(bool) {}
#include "session_render_test.h"
int main() {
    const int actions[8]={2,1,4,18,5,6,13,7};
    for(int i=0;i<8;++i)gCoOpMainOriginal[i].action=actions[i];
    CoOp_MainPage(false);
    for(i=2;i<8;++i)assert(gCoOpMainEntries[i].action==actions[i]);
    assert(gCoOpMainEntries[0].action==102 && gCoOpMainEntries[1].action==100);
    int selected=1;gCoOpMainReady=true;saveAvailable=0;held=0x1001;
    CoOp_MainNavigate(&selected);assert(selected==0);
    held=0x2002;gCoOpMainRepeat=0;CoOp_MainNavigate(&selected);assert(selected==1);
    held=0;edges=0x8008;CoOp_MainNavigate(&selected);assert(selected==5);
    edges=0x4004;CoOp_MainNavigate(&selected);assert(selected==1);
    CoOp_MainPage(true);held=edges=0x10;
    assert(!CoOp_MainTrigger(0x10,1,1) && !gCoOpMainReady);
    held=edges=0;assert(!CoOp_MainTrigger(0x10,1,1) && gCoOpMainReady);
    edges=0x10;assert(CoOp_MainTrigger(0x10,1,1));
    edges=0x20;CoOp_MainNavigate(&selected);assert(selected==2);
    edges=0;assert(CoOp_MainTrigger(0x10,1,1));assert(!CoOp_MainTrigger(0x10,1,1));
    assert(CoOp_CostumeUnlocked(1,0) && CoOp_CostumeUnlocked(8,0));
    assert(CoOp_CostumeValidated(8,0)==8 && CoOp_CostumeValidated(10,0)==8);
    assert(CoOp_CostumeNext(1,costumeUnlocks)==2);
    assert(CoOp_CostumeNext(2,costumeUnlocks)==8);
    assert(CoOp_CostumeNext(8,costumeUnlocks)==1);
    gCoOpSelectedSuit=8;
    CoOp_MainPage(2);
    assert(!strcmp(gCoOpMainEntries[2].label,"P2 COSTUME: DEFAULT"));
    costumeUnlocks=0;CoOp_MainPage(2);
    assert(gCoOpSelectedSuit==8 && !strcmp(gCoOpMainEntries[2].label,"P2 COSTUME: DEFAULT"));
    assert(CoOp_CostumeNext(8,0)==1 && CoOp_CostumeNext(1,0)==8);
    costumeUnlocks=0x83;gCoOpSelectedSuit=8;
    pages[0]=false;pages[1]=2;pages[2]=false;pages[3]=2;
    previousValues[0]=4;previousValues[1]=1;previousValues[2]=100;previousValues[3]=1;
    Sequence(100,1,0,0);replyCount=2;
    assert(CoOp_MainMenu(4)==1 && gCoOpSession.enabled && gCoOpSession.pending);
    saveAvailable=1;previousValues[1]=2;
    Sequence(100,2,0,0);assert(CoOp_MainMenu(4)==2 && gCoOpSession.enabled);
    pages[2]=2;previousValues[2]=2;
    Sequence(100,103,1,0);replyCount=3;
    assert(CoOp_MainMenu(4)==1 && gCoOpSelectedSuit==1 && costumeSaves==1);
    pages[2]=false;previousValues[2]=100;
    // An incidental shell visit between levels must preserve co-op intent.
    Sequence(100,0,0,0);replyCount=2;
    assert(CoOp_MainMenu(4)==0 && gCoOpSession.enabled && gCoOpSession.pending);
    Sequence(100,101,1,0);replyCount=3;
    assert(CoOp_MainMenu(4)==1 && !gCoOpSession.enabled);
    Sequence(100,0,0,0);replyCount=2;assert(CoOp_MainMenu(4)==0 && !gCoOpSession.enabled);
    pages[1]=1;Sequence(102,2,0,0);replyCount=2;
    assert(CoOp_MainMenu(4)==2 && !gCoOpSession.enabled);
    saveAvailable=0;previousValues[1]=1;Sequence(102,1,0,0);
    assert(CoOp_MainMenu(4)==1 && !gCoOpSession.enabled);
    Sequence(102,101,18,0);replyCount=3;previousValues[2]=102;
    assert(CoOp_MainMenu(4)==18 && !gCoOpSession.enabled);
    gCoOpSession.Select(true);gCoOpPlayers.primary=false;CoOp_RenderPlayers(0);
    assert(!gCoOpPlayers.requests && gCoOpSession.pending);
    gCoOpPlayers.primary=true;CoOp_RenderPlayers(0);
    assert(gCoOpPlayers.requests==1 && gCoOpSession.pending);
    gCoOpPlayers.canCreate=true;CoOp_RenderPlayers(0);
    assert(gCoOpPlayers.joined && !gCoOpSession.pending && gCoOpLayout==2);
    gCoOpPlayers.requests=0;gCoOpPlayers.joined=false;CoOp_RenderPlayers(0);
    assert(!gCoOpPlayers.joined && !gCoOpPlayers.requests); // manual drop-out/death
    gCoOpPlayers.replaced=true;CoOp_RenderPlayers(0);
    assert(gCoOpPlayers.joined && !gCoOpSession.pending); // new P1 lifetime
    gCoOpPlayers.requests=0;gCoOpPlayers.joined=false;CoOp_RenderPlayers(0);
    assert(!gCoOpPlayers.joined && !gCoOpPlayers.requests);
    gCoOpSession.LevelEnded();CoOp_RenderPlayers(0);
    assert(gCoOpPlayers.joined && !gCoOpSession.pending); // retry/new level
    gCoOpSession.Select(false);gCoOpSession.LevelEnded();
    assert(!gCoOpSession.enabled && !gCoOpSession.pending);
    puts("menu/session: native action routing, save availability, navigation, release gate, retry and drop-out passed");
    return 0;
}
