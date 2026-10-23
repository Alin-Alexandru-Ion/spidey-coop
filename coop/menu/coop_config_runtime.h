// Co-op / menu: Expose co-op configuration and preview through retail menu flows.
#pragma once
#include "../input/coop_config.h"
#include "../input/coop_controller_route.h"
#include "../player/coop_costume.h"
static CoOpConfig gCoOpConfig;
static bool gCoOpConfigLoaded;
static char gCoOpConfigPath[MAX_PATH];
static CMenu* gCoOpOptionsMenu;
static bool gCoOpConfigOpen,gCoOpMenuConsumed,gCoOpCaptureReady;
static bool gCoOpSaveFailed;
static int gCoOpConfigPage,gCoOpCapture=-1;
static bool gCoOpCapturePad;
static unsigned gCoOpCapturePreviousPad;
static u8 gCoOpPreviousKeys[256],gCoOpSavedMenu[0x53C];
static char gCoOpConfigLabels[6][100];
static const char* gCoOpActionNames[11]={"LEFT","RIGHT","FORWARD","BACKWARD",
    "JUMP","PUNCH","KICK","SWING","AIM","WEB","ZIPLINE"};
// Update load config in the native co-op menu flow.
static void CoOp_LoadConfig() {
    if(gCoOpConfigLoaded) return;
    gCoOpConfigLoaded=true;
    if(!GetModuleFileNameA(0,gCoOpConfigPath,MAX_PATH)) return;
    char* slash=strrchr(gCoOpConfigPath,'\\');
    if(!slash || slash-gCoOpConfigPath+20>=MAX_PATH) {gCoOpConfigPath[0]=0;return;}
    strcpy(slash+1,"coop-settings.ini");
    for(unsigned i=0;i<11;++i) {
        unsigned key=GetPrivateProfileIntA("P2",gCoOpActionNames[i],gCoOpConfig.keys[i],gCoOpConfigPath);
        if(CoOpConfig::Allowed(key)) gCoOpConfig.Bind(i,key);
        unsigned pad=GetPrivateProfileIntA("P2-PAD",gCoOpActionNames[i],gCoOpConfig.pad[i],gCoOpConfigPath);
        if(CoOp_PadMask(pad)) gCoOpConfig.BindPad(i,pad);
    }
    unsigned layout=GetPrivateProfileIntA("CO-OP","Layout",1,gCoOpConfigPath);
    gCoOpConfig.layout=layout==2?2:1;
    unsigned owner=GetPrivateProfileIntA("CO-OP","SingleControllerOwner",1,gCoOpConfigPath);
    gCoOpConfig.singleControllerOwner=owner==2?2:1;
    unsigned suit=GetPrivateProfileIntA("CO-OP","P2Costume",8,gCoOpConfigPath);
    gCoOpSelectedSuit=suit>=1 && suit<=10?suit:8;
    gCoOpLayout=(CoOpSplitLayout)gCoOpConfig.layout;
}
// Store save config in the co-op menu and saved configuration.
static bool CoOp_SaveConfig() {
    bool ok=gCoOpConfigPath[0]!=0;
    char value[20];
    for(unsigned i=0;i<11 && ok;++i) {
        sprintf(value,"%u",gCoOpConfig.keys[i]);
        ok=WritePrivateProfileStringA("P2",gCoOpActionNames[i],value,gCoOpConfigPath)!=0;
        sprintf(value,"%u",gCoOpConfig.pad[i]);
        if(ok) ok=WritePrivateProfileStringA("P2-PAD",gCoOpActionNames[i],value,gCoOpConfigPath)!=0;
    }
    sprintf(value,"%u",gCoOpConfig.singleControllerOwner);
    if(ok) ok=WritePrivateProfileStringA("CO-OP","SingleControllerOwner",value,gCoOpConfigPath)!=0;
    // Retire the former manual pad slot and P2 input-mode keys on the next save.
    if(ok) ok=WritePrivateProfileStringA("CO-OP","ControllerSlot",0,gCoOpConfigPath)!=0;
    if(ok) ok=WritePrivateProfileStringA("CO-OP","InputMode",0,gCoOpConfigPath)!=0;
    sprintf(value,"%u",gCoOpConfig.layout);
    if(ok) ok=WritePrivateProfileStringA("CO-OP","Layout",value,gCoOpConfigPath)!=0;
    sprintf(value,"%u",gCoOpSelectedSuit);
    gCoOpSaveFailed=!(ok && WritePrivateProfileStringA("CO-OP","P2Costume",value,gCoOpConfigPath)!=0);
    return !gCoOpSaveFailed;
}
// Update key name in the native co-op menu flow.
static void CoOp_KeyName(unsigned key,char* name) {
    LONG scan=(key&0x7F)<<16;if(key&0x80) scan|=1<<24;
    if(!GetKeyNameTextA(scan,name,48)) sprintf(name,"KEY %02X",key);
}
// Update config labels in the native co-op menu flow.
static void CoOp_ConfigLabels() {
    if(!gCoOpConfigOpen || !gCoOpOptionsMenu) return;
    const char* root[6]={"KEYBOARD BINDINGS","CONTROLLER BINDINGS",
        "","","RESTORE DEFAULTS","BACK"};
    for(int i=0;i<6;++i) {
        if(!gCoOpConfigPage) {
            if(i==2) strcpy(gCoOpConfigLabels[i],gCoOpConfig.singleControllerOwner==1?
                "ONE CONTROLLER: P1":"ONE CONTROLLER: P2");
            else if(i==3) strcpy(gCoOpConfigLabels[i],gCoOpConfig.layout==1?
                "DEFAULT SPLIT: VERTICAL":"DEFAULT SPLIT: HORIZONTAL");
            else strcpy(gCoOpConfigLabels[i],root[i]);
        }
        else if(gCoOpConfigPage==1 || gCoOpConfigPage==2) {
            const char* categories[6]={"MOVEMENT","ACTIONS 1","ACTIONS 2","","","BACK"};
            strcpy(gCoOpConfigLabels[i],categories[i]);
        } else {
            bool pad=gCoOpConfigPage>=6;
            int group=pad?gCoOpConfigPage-6:gCoOpConfigPage-3;
            int start=group==0?0:group==1?4:8;
            int count=group==2?3:4;
            if(i<count) {
                char key[48];
                if(pad) strcpy(key,CoOp_PadName(gCoOpConfig.pad[start+i]));
                else CoOp_KeyName(gCoOpConfig.keys[start+i],key);
                sprintf(gCoOpConfigLabels[i],"%s: %s",gCoOpActionNames[start+i],
                    gCoOpCapture==start+i && gCoOpCapturePad==pad?
                    (pad?"PRESS BUTTON (ESC CANCEL)":"PRESS KEY (ESC CANCEL)"):key);
            } else strcpy(gCoOpConfigLabels[i],i==5?"BACK":"");
        }
        gCoOpOptionsMenu->mEntry[i].name=gCoOpConfigLabels[i];
        gCoOpOptionsMenu->mEntry[i].unk_b=gCoOpConfigLabels[i][0]!=0;
    }
}
// Update config close in the native co-op menu flow.
static void CoOp_ConfigClose() {
    memcpy(gCoOpOptionsMenu,gCoOpSavedMenu,sizeof(gCoOpSavedMenu));
    gCoOpConfigOpen=false;gCoOpCapture=-1;
}
// Update config back in the native co-op menu flow.
static void CoOp_ConfigBack() {
    if(gCoOpConfigPage>=3 && gCoOpConfigPage<=5) gCoOpConfigPage=1;
    else if(gCoOpConfigPage>=6 && gCoOpConfigPage<=8) gCoOpConfigPage=2;
    else if(gCoOpConfigPage) gCoOpConfigPage=0;
    else {CoOp_ConfigClose();return;}
    gCoOpOptionsMenu->mLine=gCoOpOptionsMenu->mCursorLine=0;
    CoOp_ConfigLabels();
}
// Update config choose in the native co-op menu flow.
static void CoOp_ConfigChoose() {
    if(gCoOpMenuConsumed || gCoOpCapture>=0) return;
    gCoOpMenuConsumed=true;
    int previousPage=gCoOpConfigOpen?gCoOpConfigPage:-1;
    if(!gCoOpConfigOpen) {
        memcpy(gCoOpSavedMenu,gCoOpOptionsMenu,sizeof(gCoOpSavedMenu));
        gCoOpConfigOpen=true;gCoOpConfigPage=0;
    } else {
        int row=gCoOpOptionsMenu->mLine;
        if(row==5) {
            CoOp_ConfigBack();return;
        } else if(!gCoOpConfigPage) {
            if(row<2) gCoOpConfigPage=row+1;
            if(row==2) {gCoOpConfig.singleControllerOwner=gCoOpConfig.singleControllerOwner==1?2:1;
                CoOp_SaveConfig();}
            if(row==3) {gCoOpConfig.layout=gCoOpConfig.layout==1?2:1;
                gCoOpLayout=(CoOpSplitLayout)gCoOpConfig.layout;CoOp_SaveConfig();}
            if(row==4) {gCoOpConfig.Defaults();gCoOpLayout=COOP_SPLIT_LEFT_RIGHT;CoOp_SaveConfig();}
        } else if(gCoOpConfigPage==1 || gCoOpConfigPage==2) {
            if(row<3) gCoOpConfigPage=(gCoOpConfigPage==1?3:6)+row;
        } else {
            int group=gCoOpConfigPage>=6?gCoOpConfigPage-6:gCoOpConfigPage-3;
            int start=group==0?0:group==1?4:8;
            if(row<(group==2?3:4)) {
                gCoOpCapture=start+row;gCoOpCapturePad=gCoOpConfigPage>=6;
                gCoOpCaptureReady=false;gCoOpCapturePreviousPad=0;
            }
        }
    }
    if(previousPage!=gCoOpConfigPage)
        gCoOpOptionsMenu->mLine=gCoOpOptionsMenu->mCursorLine=0;
    CoOp_ConfigLabels();
}
static void __fastcall CoOp_OptionsAdd(CMenu* menu,void*,const char* text) {
    typedef void (__fastcall *Fn)(CMenu*,void*,const char*);
    Fn add=reinterpret_cast<Fn>(0x0043FFF0);
    add(menu,0,text);add(menu,0,"CO-OP CONFIGURATION");
    gCoOpOptionsMenu=menu;gCoOpConfigOpen=false;gCoOpCapture=-1;CoOp_LoadConfig();
}
static void __fastcall CoOp_OptionsProcess(CMenu* menu,void*) {
    gCoOpMenuConsumed=false;
    if(gCoOpCapture<0) reinterpret_cast<void (__fastcall*)(CMenu*,void*)>(0x00440600)(menu,0);
    typedef u8 (__cdecl *KeyFn)(u8);KeyFn key=reinterpret_cast<KeyFn>(0x00501CB0);
    bool any=false;
    unsigned connectedSlots=0,padMasks[4]={0,0,0,0};
    for(unsigned slot=0;slot<4;++slot) {
        bool connected=false;
        padMasks[slot]=CoOp_ReadGamepad(slot,&connected);
        if(connected)connectedSlots|=1u<<slot;
    }
    int captureSlot=CoOp_BindingCaptureSlot(connectedSlots);
    unsigned pad=captureSlot>=0?padMasks[captureSlot]:0;
    for(int k=1;k<256;++k) {
        u8 down=key((u8)k)!=0;any|=(down!=0);
        if(gCoOpCapture>=0 && gCoOpCaptureReady && down && !gCoOpPreviousKeys[k]) {
            if(k==1) {gCoOpCapture=-1;gCoOpMenuConsumed=true;}
            else if(!gCoOpCapturePad) {
                char name[48];LONG scan=(k&0x7F)<<16;if(k&0x80) scan|=1<<24;
                if(GetKeyNameTextA(scan,name,sizeof(name)) && gCoOpConfig.Bind(gCoOpCapture,k)) {
                    CoOp_SaveConfig();gCoOpCapture=-1;gCoOpMenuConsumed=true;
                }
            }
        }
        gCoOpPreviousKeys[k]=down;
    }
    if(gCoOpCapture>=0 && gCoOpCapturePad && gCoOpCaptureReady) {
        unsigned pressed=pad&~gCoOpCapturePreviousPad;
        for(unsigned code=1;code<=24;++code) if(pressed&CoOp_PadMask(code)) {
            if(gCoOpConfig.BindPad(gCoOpCapture,code)) {
                CoOp_SaveConfig();gCoOpCapture=-1;gCoOpMenuConsumed=true;
            }
            break;
        }
    }
    gCoOpCapturePreviousPad=pad;
    if(!any && (!gCoOpCapturePad || !pad)) gCoOpCaptureReady=true;
    CoOp_ConfigLabels();
}
// Update options trigger in the native co-op menu flow.
static u8 __cdecl CoOp_OptionsTrigger(u32 mask,i32 a,i32 b) {
    u8 result=reinterpret_cast<u8 (__cdecl*)(u32,i32,i32)>(0x0050C180)(mask,a,b);
    if(gCoOpMenuConsumed || gCoOpCapture>=0) return 0;
    if(!result) return 0;
    if(mask==0x20220) {
        if(!gCoOpConfigOpen) return result;
        gCoOpMenuConsumed=true;
        CoOp_ConfigBack();
        return 0;
    }
    if(mask==0x100) return result; // mouse hit-test follows
    if(gCoOpConfigOpen || gCoOpOptionsMenu->mLine==5) {CoOp_ConfigChoose();return 0;}
    return result;
}
// Update options mouse in the native co-op menu flow.
static u8 __cdecl CoOp_OptionsMouse(const char* text,i32 x,i32 y,i32 justification) {
    u8 result=reinterpret_cast<u8 (__cdecl*)(const char*,i32,i32,i32)>(0x0050C6C0)(text,x,y,justification);
    if(gCoOpMenuConsumed || gCoOpCapture>=0) return 0;
    if(result && (gCoOpConfigOpen || gCoOpOptionsMenu->mLine==5)) {CoOp_ConfigChoose();return 0;}
    return result;
}
static int __fastcall CoOp_OptionsZoomed(CMenu* menu,void*) {
    return gCoOpConfigOpen?0:reinterpret_cast<int (__fastcall*)(CMenu*,void*)>(0x0043FE60)(menu,0);
}
// Update options title in the native co-op menu flow.
static void __cdecl CoOp_OptionsTitle(int x,int y,const char* title,int a,int b,int c,int d,int e) {
    reinterpret_cast<void (__cdecl*)(int,int,const char*,int,int,int,int,int)>(0x0048D9C0)
        (x,y,gCoOpConfigOpen?(gCoOpSaveFailed?"CO-OP: SAVE FAILED":"CO-OP CONFIGURATION"):title,a,b,c,d,e);
}
// Install install config hooks at the supported retail call site, preserving its fallback.
static bool CoOp_InstallConfigHooks() {
    const u32 sites[]={0x49716C,0x497402,0x497410,0x49742B,0x497496,0x497471,0x497294,0x497283};
    const u32 targets[]={0x43FFF0,0x440600,0x50C180,0x50C180,0x50C180,0x50C6C0,0x43FE60,0x48D9C0};
    const u32 hooks[]={reinterpret_cast<u32>(&CoOp_OptionsAdd),reinterpret_cast<u32>(&CoOp_OptionsProcess),
        reinterpret_cast<u32>(&CoOp_OptionsTrigger),reinterpret_cast<u32>(&CoOp_OptionsTrigger),
        reinterpret_cast<u32>(&CoOp_OptionsTrigger),reinterpret_cast<u32>(&CoOp_OptionsMouse),
        reinterpret_cast<u32>(&CoOp_OptionsZoomed),reinterpret_cast<u32>(&CoOp_OptionsTitle)};
    if(sizeof(CMenu)!=0x53C) return false;
    for(unsigned i=0;i<8;++i) {
        const u8* code=reinterpret_cast<const u8*>(sites[i]);
        if(code[0]!=0xE8 || sites[i]+5+*reinterpret_cast<const i32*>(code+1)!=targets[i]) return false;
    }
    for(i=0;i<8;++i) {
        DWORD old,unused;
        if(!VirtualProtect(reinterpret_cast<void*>(sites[i]),5,PAGE_EXECUTE_READWRITE,&old)) return false;
        *reinterpret_cast<i32*>(sites[i]+1)=hooks[i]-sites[i]-5;
        VirtualProtect(reinterpret_cast<void*>(sites[i]),5,old,&unused);
        FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(sites[i]),5);
    }
    return true;
}
