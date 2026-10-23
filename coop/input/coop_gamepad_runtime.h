// Co-op / input: Route keyboard/gamepad input and settings to the intended player.
#pragma once
#include "coop_gamepad.h"

struct CoOpXInputPad {
    unsigned short buttons;
    unsigned char leftTrigger,rightTrigger;
    short lx,ly,rx,ry;
};
struct CoOpXInputState {unsigned long packet;CoOpXInputPad pad;};
typedef unsigned long (WINAPI *CoOpXInputGetStateFn)(unsigned long,CoOpXInputState*);
static CoOpXInputGetStateFn gCoOpXInputGetState;
static bool gCoOpXInputChecked,gCoOpReservePrimaryController;
static unsigned gCoOpPadPreviousMask;
// Return read gamepad from the selected player's bindings and device state.
static unsigned CoOp_ReadGamepad(unsigned slot,bool* connected) {
    if(connected) *connected=false;
    if(slot>3) return 0;
    if(!gCoOpXInputChecked) {
        gCoOpXInputChecked=true;
        const char* libraries[]={"xinput1_4.dll","xinput1_3.dll","xinput9_1_0.dll"};
        for(unsigned i=0;i<3 && !gCoOpXInputGetState;++i) {
            HMODULE module=LoadLibraryA(libraries[i]);
            if(module) gCoOpXInputGetState=reinterpret_cast<CoOpXInputGetStateFn>(GetProcAddress(module,"XInputGetState"));
        }
    }
    if(!gCoOpXInputGetState) return 0;
    CoOpXInputState state;memset(&state,0,sizeof(state));
    if(gCoOpXInputGetState(slot,&state)!=0) return 0;
    if(connected) *connected=true;
    unsigned mask=0;
    const unsigned short bits[]={0x0001,0x0002,0x0004,0x0008,0x0010,0x0020,
        0x0040,0x0080,0x0100,0x0200,0x1000,0x2000,0x4000,0x8000};
    for(unsigned i=0;i<14;++i) if(state.pad.buttons&bits[i]) mask|=CoOp_PadMask(i+1);
    if(state.pad.leftTrigger>30) mask|=CoOp_PadMask(COOP_PAD_LT);
    if(state.pad.rightTrigger>30) mask|=CoOp_PadMask(COOP_PAD_RT);
    const int deadzone=12000;
    if(state.pad.ly>deadzone) mask|=CoOp_PadMask(COOP_PAD_LS_UP);
    if(state.pad.ly< -deadzone) mask|=CoOp_PadMask(COOP_PAD_LS_DOWN);
    if(state.pad.lx< -deadzone) mask|=CoOp_PadMask(COOP_PAD_LS_LEFT);
    if(state.pad.lx>deadzone) mask|=CoOp_PadMask(COOP_PAD_LS_RIGHT);
    if(state.pad.ry>deadzone) mask|=CoOp_PadMask(COOP_PAD_RS_UP);
    if(state.pad.ry< -deadzone) mask|=CoOp_PadMask(COOP_PAD_RS_DOWN);
    if(state.pad.rx< -deadzone) mask|=CoOp_PadMask(COOP_PAD_RS_LEFT);
    if(state.pad.rx>deadzone) mask|=CoOp_PadMask(COOP_PAD_RS_RIGHT);
    return mask;
}
// The original mapper also samples its one DirectInput controller for P1.
// In a co-op session a connected pad reserved for P2 must not move P1 too.
static i32 __cdecl CoOp_PollPrimaryController(i32* x,i32* y,i32* extra) {
    return gCoOpReservePrimaryController?0:
        reinterpret_cast<i32 (__cdecl*)(i32*,i32*,i32*)>(0x00501E50)(x,y,extra);
}
// Install install gamepad hook at the supported retail call site, preserving its fallback.
static bool CoOp_InstallGamepadHook() {
    const u32 site=0x0050A2AD;
    const u8* code=reinterpret_cast<const u8*>(site);
    if(code[0]!=0xE8 || site+5+*reinterpret_cast<const i32*>(code+1)!=0x00501E50) return false;
    DWORD old,unused;
    if(!VirtualProtect(reinterpret_cast<void*>(site),5,PAGE_EXECUTE_READWRITE,&old)) return false;
    *reinterpret_cast<i32*>(site+1)=reinterpret_cast<u32>(&CoOp_PollPrimaryController)-site-5;
    VirtualProtect(reinterpret_cast<void*>(site),5,old,&unused);
    FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(site),5);
    return true;
}
