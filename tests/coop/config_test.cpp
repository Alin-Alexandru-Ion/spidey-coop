#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "coop/camera/coop_view.h"
typedef unsigned char u8;
typedef unsigned long u32;
typedef long i32;
static u32 costumeUnlocks;
static void* Address(u32 a) {assert(a==0x006828D8);return &costumeUnlocks;}
struct Entry {const char* name;char pad[9];u8 unk_b;char tail[18];};
struct CMenu {char head[20];u8 mLine,mCursorLine;char middle[4];u8 mNumLines;char rest[33];Entry mEntry[40];};
static CoOpSplitLayout gCoOpLayout=COOP_SPLIT_LEFT_RIGHT;
static u8 heldKeys[256];
static u8 trigger;
static unsigned testPad;
static unsigned connectedSlots=1,padSlot=0;
static unsigned CoOp_ReadGamepad(unsigned slot,bool* connected) {
    *connected=(connectedSlots&(1u<<slot))!=0;
    return *connected && slot==padSlot?testPad:0;
}
static void __fastcall Add(CMenu* m,void*,const char* s) {m->mEntry[m->mNumLines++].name=s;}
static void __fastcall Process(CMenu*,void*) {}
static u8 __cdecl Key(u8 k) {return heldKeys[k];}
static u8 __cdecl Trigger(u32,int,int) {return trigger;}
static u8 __cdecl Mouse(const char*,int,int,int) {return trigger;}
static int __fastcall Zoomed(CMenu*,void*) {return 1;}
static void __cdecl Title(int,int,const char*,int,int,int,int,int) {}
#include "config_runtime_test.h"
static void Select(CMenu& menu,unsigned row) {
    CoOp_OptionsProcess(&menu,0);menu.mLine=menu.mCursorLine=(u8)row;trigger=1;
    assert(CoOp_OptionsTrigger(0x10010,1,1)==0);trigger=0;
}
int main() {
    assert(sizeof(CMenu)==0x53C && sizeof(Entry)==32);
    CMenu menu;memset(&menu,0,sizeof(menu));menu.mNumLines=4;
    CoOp_OptionsAdd(&menu,0,"DISPLAY");
    assert(menu.mNumLines==6 && !strcmp(menu.mEntry[5].name,"CO-OP CONFIGURATION"));
    assert(CoOpConfig::Allowed(0x0B) && CoOpConfig::Allowed(0x07));
    assert(!CoOpConfig::Allowed(0x33) && !CoOpConfig::Allowed(0xCF));
    CoOpConfig defaults;
    assert(defaults.keys[4]==0x4F && defaults.layout==1);
    assert(defaults.singleControllerOwner==1);
    assert(defaults.pad[4]==COOP_PAD_A && defaults.pad[9]==COOP_PAD_Y);
    assert(CoOp_PadDown(CoOp_PadMask(COOP_PAD_A),COOP_PAD_A));
    assert(!defaults.BindPad(0,25));
    assert(defaults.BindPad(4,COOP_PAD_X) && defaults.pad[5]==COOP_PAD_A);
    assert(!defaults.Bind(0,0xCF) && !defaults.Bind(0,0x1C));
    assert(defaults.Bind(0,0xCD) && defaults.keys[1]==0xCB);
    Select(menu,5);assert(gCoOpConfigOpen && !CoOp_OptionsZoomed(&menu,0));
    Select(menu,0);assert(gCoOpConfigPage==1);
    Select(menu,0);assert(gCoOpConfigPage==3);
    heldKeys[0x1C]=1;Select(menu,0);assert(gCoOpCapture==0 && !gCoOpCaptureReady);
    CoOp_OptionsProcess(&menu,0);assert(gCoOpCapture==0);
    heldKeys[0x1C]=0;CoOp_OptionsProcess(&menu,0);assert(gCoOpCaptureReady);
    heldKeys[0x1E]=1;CoOp_OptionsProcess(&menu,0);
    assert(gCoOpCapture==-1 && gCoOpConfig.keys[0]==0x1E && !gCoOpSaveFailed);
    CoOp_OptionsProcess(&menu,0);
    trigger=1;assert(CoOp_OptionsTrigger(0x20220,1,1)==0 && gCoOpConfigPage==1);
    heldKeys[0x1E]=0;trigger=0;
    Select(menu,0);assert(gCoOpConfigPage==3);
    Select(menu,1);CoOp_OptionsProcess(&menu,0);
    heldKeys[1]=1;CoOp_OptionsProcess(&menu,0);
    assert(gCoOpCapture==-1 && gCoOpConfig.keys[1]==0xCD);
    heldKeys[1]=0;
    Select(menu,5);assert(gCoOpConfigPage==1);
    Select(menu,5);assert(gCoOpConfigPage==0);
    Select(menu,1);assert(gCoOpConfigPage==2);
    Select(menu,1);assert(gCoOpConfigPage==7);
    Select(menu,0);assert(gCoOpCapture==4 && gCoOpCapturePad);
    CoOp_OptionsProcess(&menu,0);assert(gCoOpCapture==4);
    testPad=CoOp_PadMask(COOP_PAD_LB);CoOp_OptionsProcess(&menu,0);
    assert(gCoOpCapture==-1 && gCoOpConfig.pad[4]==COOP_PAD_LB);
    testPad=0;Select(menu,5);Select(menu,5);
    assert(gCoOpConfigPage==0);
    assert(!strcmp(menu.mEntry[2].name,"ONE CONTROLLER: P1"));
    Select(menu,2);assert(gCoOpConfig.singleControllerOwner==2);
    assert(!strcmp(menu.mEntry[2].name,"ONE CONTROLLER: P2"));
    Select(menu,3);assert(gCoOpConfig.layout==2 && gCoOpLayout==COOP_SPLIT_TOP_BOTTOM);
    assert(!strcmp(menu.mEntry[3].name,"DEFAULT SPLIT: HORIZONTAL"));
    // Binding capture uses the second connected pad, even with sparse slots.
    connectedSlots=(1u<<0)|(1u<<2);padSlot=2;
    Select(menu,1);Select(menu,1);Select(menu,0);
    assert(gCoOpCapture==4 && gCoOpCapturePad);
    CoOp_OptionsProcess(&menu,0);testPad=CoOp_PadMask(COOP_PAD_B);
    CoOp_OptionsProcess(&menu,0);
    assert(gCoOpCapture==-1 && gCoOpConfig.pad[4]==COOP_PAD_B);
    testPad=0;connectedSlots=1;padSlot=0;
    Select(menu,5);Select(menu,5);
    Select(menu,5);assert(!gCoOpConfigOpen && !strcmp(menu.mEntry[5].name,"CO-OP CONFIGURATION"));
    gCoOpSelectedSuit=7;assert(CoOp_SaveConfig());
    gCoOpConfig.Defaults();gCoOpConfigLoaded=false;CoOp_LoadConfig();
    assert(gCoOpConfig.keys[0]==0x1E && gCoOpConfig.layout==2);
    assert(gCoOpConfig.pad[4]==COOP_PAD_B);
    assert(gCoOpConfig.singleControllerOwner==2);
    assert(gCoOpSelectedSuit==7);
    WritePrivateProfileStringA("P2","LEFT","207",gCoOpConfigPath);
    WritePrivateProfileStringA("CO-OP","Layout","999",gCoOpConfigPath);
    WritePrivateProfileStringA("CO-OP","ControllerSlot","999",gCoOpConfigPath);
    WritePrivateProfileStringA("CO-OP","InputMode","2",gCoOpConfigPath);
    WritePrivateProfileStringA("CO-OP","SingleControllerOwner","999",gCoOpConfigPath);
    WritePrivateProfileStringA("P2-PAD","JUMP","999",gCoOpConfigPath);
    WritePrivateProfileStringA("CO-OP","P2Costume","999",gCoOpConfigPath);
    gCoOpConfig.Defaults();gCoOpConfigLoaded=false;CoOp_LoadConfig();
    assert(gCoOpConfig.keys[0]==0xCB && gCoOpConfig.layout==1);
    assert(gCoOpConfig.pad[4]==COOP_PAD_A);
    assert(gCoOpConfig.singleControllerOwner==1);
    assert(GetPrivateProfileIntA("CO-OP","ControllerSlot",77,gCoOpConfigPath)==999);
    assert(GetPrivateProfileIntA("CO-OP","InputMode",77,gCoOpConfigPath)==2);
    assert(CoOp_SaveConfig());
    assert(GetPrivateProfileIntA("CO-OP","ControllerSlot",77,gCoOpConfigPath)==77);
    assert(GetPrivateProfileIntA("CO-OP","InputMode",77,gCoOpConfigPath)==77);
    assert(gCoOpSelectedSuit==8);
    DeleteFileA(gCoOpConfigPath);
    puts("PASS: keyboard/pad remap, automatic second-pad capture, one-controller owner, layout and retired keys");
    return 0;
}
