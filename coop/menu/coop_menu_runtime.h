// Co-op / menu: Expose co-op configuration and preview through retail menu flows.
#pragma once
#include "../core/coop_session.h"
#include "../player/coop_costume.h"
static CoOpSession gCoOpSession;
struct CoOpMainEntry {int x,y;const char* label;int action;u32 preview[6];};
static CoOpMainEntry gCoOpMainOriginal[8],gCoOpMainEntries[9];
static char gCoOpCostumeLabel[64];
static int gCoOpMainSubmenu; // 0=root, 1=singleplayer, 2=co-op
static bool gCoOpMainReady,gCoOpMainBack;
static unsigned gCoOpMainRepeat;
static const int COOP_MENU_ACTION=100,COOP_MENU_BACK=101,SINGLE_MENU_ACTION=102,
    COOP_MENU_COSTUME=103;
static int gCoOpMainCount=8;

// Update main key in the native co-op menu flow.
static u8 CoOp_MainKey(u32 mask,int edge) {
    return reinterpret_cast<u8 (__cdecl*)(u32,int,int)>(0x0050C180)(mask,edge,edge);
}
// Update main navigate in the native co-op menu flow.
static void CoOp_MainNavigate(int* selection) {
    int old=*selection, next=old;
    if(!gCoOpMainReady) return;
    if(gCoOpMainSubmenu && CoOp_MainKey(0x20220,1)) {
        *selection=2;gCoOpMainBack=true;return;
    }
    if(CoOp_MainKey(0x3003,0)) {
        if(!gCoOpMainRepeat || (gCoOpMainRepeat>20 && !(gCoOpMainRepeat&1))) {
            int direction=CoOp_MainKey(0x1001,0)?-1:1;
            do {next=(next+direction+gCoOpMainCount)%gCoOpMainCount;}
            while(gCoOpMainSubmenu && next==0 && !*reinterpret_cast<u8*>(0x006828D0));
        }
        ++gCoOpMainRepeat;
    } else gCoOpMainRepeat=0;
    if(!gCoOpMainSubmenu) {
        if(CoOp_MainKey(0x8008,1) && next<4) next+=4;
        if(CoOp_MainKey(0x4004,1) && next>=4) next-=4;
    }
    if(gCoOpMainSubmenu && next==0 && !*reinterpret_cast<u8*>(0x006828D0)) next=1;
    *selection=next;
    if(old!=next) reinterpret_cast<void (__cdecl*)(int,int,int)>(0x00471C50)(0x29,0x3FFF,0);
}
// Replaces only the native navigation block. The outer shell owns this stack:
// selection is [esp+40h], and EBP carries it into the preview-update block.
static void __declspec(naked) CoOp_MainNavigationBridge() {
    __asm {
        lea eax,[esp+40h]
        pushad
        push eax
        call CoOp_MainNavigate
        add esp,4
        popad
        mov ebp,[esp+40h]
        push 0049404Eh
        ret
    }
}
// Update main trigger in the native co-op menu flow.
static u8 __cdecl CoOp_MainTrigger(u32 mask,int a,int b) {
    u8 result=reinterpret_cast<u8 (__cdecl*)(u32,int,int)>(0x0050C180)(mask,a,b);
    if(!gCoOpMainReady) {
        // Enter/click opening this page must be released before its first choice.
        if(!CoOp_MainKey(0x110,0) && !CoOp_MainKey(0x20220,0)) gCoOpMainReady=true;
        return 0;
    }
    if(mask==0x10 && gCoOpMainBack) {gCoOpMainBack=false;return 1;}
    return result;
}
// Update main cursor in the native co-op menu flow.
static void __cdecl CoOp_MainCursor() {
    if(gCoOpMainSubmenu) {
        reinterpret_cast<void (__cdecl*)(int,int,const char*,int,int)>(0x00458700)
            (256,22,gCoOpMainSubmenu==2?"CO-OP":"SINGLEPLAYER",0,0x1000);
    }
    reinterpret_cast<void (__cdecl*)()>(0x0050C470)();
}
// All pointers used by the shell AND its animated preview are relocated.
// End pointers/counts are changed together when entering the three-row page.
static const u32 gCoOpMainPointerSites[]={
    0x493890,0x4938A3,0x4938B2,0x493934,0x493940,0x49394D,0x493958,
    0x4939CC,0x493BF5,0x493C49,0x493D04,0x493D8F,0x493D95,0x493DAD,
    0x493E2D,0x493E33,0x493E39,0x493E6D,0x493ECB,0x494055,0x494269};
static const u32 gCoOpMainPointerValues[]={
    0x552ABC,0x552AB8,0x552ABC,0x552AC8,0x552ACC,0x552AC0,0x552AC4,
    0x552AB4,0x552AB4,0x552AB4,0x552AA8,0x552AB0,0x552AAC,0x552AA8,
    0x552AAC,0x552AA8,0x552AB0,0x552AB4,0x552AA8,0x552AB4,0x552AB4};
// Update main page in the native co-op menu flow.
static void CoOp_MainPage(int submenu) {
    gCoOpMainSubmenu=submenu;gCoOpMainCount=submenu==2?4:submenu?3:8;
    gCoOpMainReady=false;gCoOpMainBack=false;gCoOpMainRepeat=0;
    for(int i=0;i<8;++i) gCoOpMainEntries[i]=gCoOpMainOriginal[i];
    gCoOpMainEntries[0]=gCoOpMainOriginal[1];
    gCoOpMainEntries[0].x=105;gCoOpMainEntries[0].y=46;
    gCoOpMainEntries[0].label="SINGLEPLAYER";gCoOpMainEntries[0].action=SINGLE_MENU_ACTION;
    gCoOpMainEntries[1].label="CO-OP";gCoOpMainEntries[1].action=COOP_MENU_ACTION;
    if(submenu) {
        gCoOpMainEntries[0]=gCoOpMainOriginal[0];
        gCoOpMainEntries[1]=gCoOpMainOriginal[1];
        for(i=0;i<2;++i) {gCoOpMainEntries[i].x=105;gCoOpMainEntries[i].y=(submenu==2?46:70)+i*48;}
        if(submenu==2) {
            gCoOpSelectedSuit=CoOp_CostumeValidated(gCoOpSelectedSuit,CoOp_CostumeUnlockMask());
            sprintf(gCoOpCostumeLabel,"P2 COSTUME: %s",gCoOpSuitNames[gCoOpSelectedSuit]);
            gCoOpMainEntries[2]=gCoOpMainOriginal[3];
            gCoOpMainEntries[2].x=105;gCoOpMainEntries[2].y=142;
            gCoOpMainEntries[2].label=gCoOpCostumeLabel;
            gCoOpMainEntries[2].action=COOP_MENU_COSTUME;
        }
        int back=submenu==2?3:2;
        gCoOpMainEntries[back]=gCoOpMainOriginal[3];
        gCoOpMainEntries[back].x=105;gCoOpMainEntries[back].y=submenu==2?190:166;
        gCoOpMainEntries[back].label="BACK";gCoOpMainEntries[back].action=COOP_MENU_BACK;
    }
    DWORD old,unused;
    if(VirtualProtect(reinterpret_cast<void*>(0x493860),0xA20,PAGE_EXECUTE_READWRITE,&old)) {
        *reinterpret_cast<u32*>(0x4939D9)=reinterpret_cast<u32>(gCoOpMainEntries)+gCoOpMainCount*40+12;
        *reinterpret_cast<u32*>(0x493EF0)=reinterpret_cast<u32>(gCoOpMainEntries)+gCoOpMainCount*40;
        *reinterpret_cast<u16*>(0x493C12)=submenu==2?0x9090:0x0974; // keep both previews on disabled Continue
        *reinterpret_cast<u8*>(0x493D08)=submenu?0x75:0xEB; // root SINGLEPLAYER is always available
        *reinterpret_cast<u8*>(0x49386E)=(u8)gCoOpMainCount;
        *reinterpret_cast<u8*>(0x493D73)=(u8)gCoOpMainCount;
        VirtualProtect(reinterpret_cast<void*>(0x493860),0xA20,old,&unused);
        FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(0x493860),0xA20);
    }
}
// Update main menu in the native co-op menu flow.
static int __cdecl CoOp_MainMenu(int previous) {
    // The shell can revisit this page between levels. Keep the active run's
    // mode until a player explicitly starts a single-player or co-op game.
    CoOp_LoadConfig();
    int submenu=0;
    for(;;) {
        CoOp_MainPage(submenu);
        int result=reinterpret_cast<int (__cdecl*)(int)>(0x00493990)(previous);
        if(!submenu && (result==COOP_MENU_ACTION || result==SINGLE_MENU_ACTION)) {
            submenu=result==COOP_MENU_ACTION?2:1;previous=*reinterpret_cast<u8*>(0x006828D0)?2:1;continue;
        }
        if(submenu && result==COOP_MENU_BACK) {
            previous=submenu==2?COOP_MENU_ACTION:SINGLE_MENU_ACTION;submenu=0;continue;
        }
        if(submenu==2 && result==COOP_MENU_COSTUME) {
            gCoOpSelectedSuit=CoOp_CostumeNext(gCoOpSelectedSuit,CoOp_CostumeUnlockMask());
            CoOp_SaveConfig();previous=2;continue;
        }
        if(result==1 || result==2) gCoOpSession.Select(submenu==2);
        return result; // unchanged retail shell action: difficulty/save/level flow
    }
}
#include "coop_menu_preview.h"
// Install install main menu hooks at the supported retail call site, preserving its fallback.
static bool CoOp_InstallMainMenuHooks() {
    if(sizeof(CoOpMainEntry)!=40 || sizeof(CDummy)!=0xA18) return false;
    unsigned i;
    for(i=0;i<sizeof(gCoOpMainPointerSites)/sizeof(u32);++i)
        if(*reinterpret_cast<u32*>(gCoOpMainPointerSites[i])!=gCoOpMainPointerValues[i]) return false;
    const u32 sites[]={0x4A202F,0x493E1F,0x493E56,0x493DBE,0x493B3E,0x493C15,0x494142};
    const u32 targets[]={0x493990,0x50C180,0x50C180,0x50C470,0x490DF0,0x4739A0,0x411B30};
    const u32 hooks[]={reinterpret_cast<u32>(&CoOp_MainMenu),reinterpret_cast<u32>(&CoOp_MainTrigger),
        reinterpret_cast<u32>(&CoOp_MainTrigger),reinterpret_cast<u32>(&CoOp_MainCursor),reinterpret_cast<u32>(&CoOp_MenuDummyCtor),
        reinterpret_cast<u32>(&CoOp_MenuDrawPreview),reinterpret_cast<u32>(&CoOp_MenuUpdateEffects)};
    for(i=0;i<7;++i) {
        const u8* c=reinterpret_cast<const u8*>(sites[i]);
        if(c[0]!=0xE8 || sites[i]+5+*reinterpret_cast<const i32*>(c+1)!=targets[i]) return false;
    }
    const u8 nav[]={0x6A,0,0x6A,0,0x68};
    if(memcmp(reinterpret_cast<void*>(0x493EFA),nav,5) ||
       *reinterpret_cast<u32*>(0x4939D9)!=0x552BF4 || *reinterpret_cast<u32*>(0x493EF0)!=0x552BE8 ||
       *reinterpret_cast<u8*>(0x49386E)!=8 || *reinterpret_cast<u8*>(0x493D73)!=8 ||
       *reinterpret_cast<u8*>(0x493D89)!=4 || *reinterpret_cast<u8*>(0x493D08)!=0x75 || *reinterpret_cast<u16*>(0x493C12)!=0x0974) return false;
    memcpy(gCoOpMainOriginal,reinterpret_cast<void*>(0x552AA8),sizeof(gCoOpMainOriginal));
    DWORD old,callOld,unused;
    if(!VirtualProtect(reinterpret_cast<void*>(0x493860),0xA20,PAGE_EXECUTE_READWRITE,&old)) return false;
    if(!VirtualProtect(reinterpret_cast<void*>(0x4A202F),5,PAGE_EXECUTE_READWRITE,&callOld)) {
        VirtualProtect(reinterpret_cast<void*>(0x493860),0xA20,old,&unused);return false;
    }
    for(i=0;i<sizeof(gCoOpMainPointerSites)/sizeof(u32);++i)
        *reinterpret_cast<u32*>(gCoOpMainPointerSites[i])=reinterpret_cast<u32>(gCoOpMainEntries)+gCoOpMainPointerValues[i]-0x552AA8;
    *reinterpret_cast<u8*>(0x493D89)=4; // original four-row columns
    *reinterpret_cast<u8*>(0x493EFA)=0xE9;
    *reinterpret_cast<u32*>(0x493EFB)=reinterpret_cast<u32>(&CoOp_MainNavigationBridge)-0x493EFF;
    for(i=0;i<7;++i) *reinterpret_cast<u32*>(sites[i]+1)=hooks[i]-sites[i]-5;
    VirtualProtect(reinterpret_cast<void*>(0x493860),0xA20,old,&unused);
    VirtualProtect(reinterpret_cast<void*>(0x4A202F),5,callOld,&unused);
    FlushInstructionCache(GetCurrentProcess(),0,0);
    CoOp_MainPage(false);return true;
}
