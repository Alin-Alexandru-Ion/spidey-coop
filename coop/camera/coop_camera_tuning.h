// Co-op / camera: Select and render independent camera behavior in the shared world.
#pragma once
// SetSpideyCamValue is authored camera tuning, distinct from actor-owned
// cinematic commands. Replicate its updates, including those after early join.
struct CoOpCameraSetting {u16 mode,field;short value;u16 duration,immediate;unsigned pending;};
static CoOpCameraSetting gCoOpCameraSettings[30];
// Apply camera setting to the selected camera without changing the other view.
static void CoOp_ApplyCameraSetting(void* argument) {
    CoOpCameraSetting* c=static_cast<CoOpCameraSetting*>(argument);
    CPlayer* player=*reinterpret_cast<CPlayer**>(0x006A9038);
    if(player) reinterpret_cast<void (__fastcall*)(CPlayer*,void*,u16,u16,short,u16,u16)>(0x004B97D0)
        (player,0,c->mode,c->field,c->value,c->duration,c->immediate);
}
static void __fastcall CoOp_SetCameraSetting(CPlayer* player,void*,u16 mode,u16 field,short value,u16 duration,u16 immediate) {
    reinterpret_cast<void (__fastcall*)(CPlayer*,void*,u16,u16,short,u16,u16)>(0x004B97D0)
        (player,0,mode,field,value,duration,immediate);
    if(mode<6 && field<5) {
        CoOpCameraSetting& c=gCoOpCameraSettings[mode*5+field];
        c.mode=mode;c.field=field;c.value=value;c.duration=duration;c.immediate=immediate;
        c.pending=gCoOpPlayers.InSecondaryContext()?1:2;
    }
}
// Update flush camera settings in the selected camera's private state.
static void CoOp_FlushCameraSettings() {
    for(unsigned i=0;i<30;++i) {
        CoOpCameraSetting& c=gCoOpCameraSettings[i];
        if((c.pending&1) && gCoOpPlayers.Get(COOP_PLAYER_ONE)) {CoOp_ApplyCameraSetting(&c);c.pending&=~1u;}
        if((c.pending&2) && gCoOpPlayers.WithSecondaryContext(CoOp_ApplyCameraSetting,&c)) c.pending&=~2u;
    }
}
// Install install camera tuning hook at the supported retail call site, preserving its fallback.
static bool CoOp_InstallCameraTuningHook() {
    u8* site=reinterpret_cast<u8*>(0x004E2515);
    if(site[0]!=0xE8 || 0x004E251A+*reinterpret_cast<i32*>(site+1)!=0x004B97D0) return false;
    DWORD old,unused;if(!VirtualProtect(site,5,PAGE_EXECUTE_READWRITE,&old)) return false;
    *reinterpret_cast<u32*>(site+1)=reinterpret_cast<u32>(&CoOp_SetCameraSetting)-0x004E251A;
    VirtualProtect(site,5,old,&unused);FlushInstructionCache(GetCurrentProcess(),site,5);return true;
}
