// Co-op / camera: Select and render independent camera behavior in the shared world.

#ifndef COOP_CAMERA_POLICY_RUNTIME_H
#define COOP_CAMERA_POLICY_RUNTIME_H
// Script opcodes 160/187 describe shared camera rules, not a camera pose.
// Apply only their native setters to each participant; never replay the scene.
struct CoOpCameraPolicy {
    ECameraMode mode;u16 locked;unsigned modePending,lockPending;
    // Initialize co-op camera policy with no active retail ownership.
    CoOpCameraPolicy():mode(CAMERAMODE_DEMO),locked(0),modePending(0),lockPending(0) {}
};
static CoOpCameraPolicy gCoOpCameraPolicy;
static bool gCoOpCameraPolicyInstalled;
static unsigned gCoOpCameraPolicyModes[2],gCoOpCameraPolicyLocks[2];
// Apply camera policy to the selected camera without changing the other view.
static void CoOp_ApplyCameraPolicy(void* argument) {
    unsigned slot=*static_cast<unsigned*>(argument);
    CCamera* camera=*reinterpret_cast<CCamera**>(0x0056F3B8);
    CPlayer* player=*reinterpret_cast<CPlayer**>(0x006A9038);
    unsigned mask=1u<<slot;
    if(camera && (gCoOpCameraPolicy.modePending&mask)) {
        reinterpret_cast<i32 (__fastcall*)(CCamera*,void*,ECameraMode)>(0x004167F0)(
            camera,0,gCoOpCameraPolicy.mode);
        gCoOpCameraPolicy.modePending&=~mask;++gCoOpCameraPolicyModes[slot];
    }
    if(player && (gCoOpCameraPolicy.lockPending&mask)) {
        reinterpret_cast<void (__fastcall*)(CPlayer*,void*,u16)>(0x004B9E10)(
            player,0,gCoOpCameraPolicy.locked);
        gCoOpCameraPolicy.lockPending&=~mask;++gCoOpCameraPolicyLocks[slot];
    }
}
static i32 __fastcall CoOp_ScriptCameraMode(CCamera* camera,void*,ECameraMode mode) {
    i32 previous=reinterpret_cast<i32 (__fastcall*)(CCamera*,void*,ECameraMode)>(0x004167F0)(camera,0,mode);
    if(gCoOpSession.enabled && camera==*reinterpret_cast<CCamera**>(0x0056F3B8)) {
        unsigned slot=gCoOpPlayers.InSecondaryContext()?1:0;
        gCoOpCameraPolicy.mode=mode;
        // The latest authored rule supersedes any not-yet-applied rule.
        gCoOpCameraPolicy.modePending=1u<<(1-slot);++gCoOpCameraPolicyModes[slot];
    }
    return previous;
}
static void __fastcall CoOp_ScriptCameraLock(CPlayer* player,void*,u16 locked) {
    reinterpret_cast<void (__fastcall*)(CPlayer*,void*,u16)>(0x004B9E10)(player,0,locked);
    if(gCoOpSession.enabled && player==*reinterpret_cast<CPlayer**>(0x006A9038)) {
        unsigned slot=gCoOpPlayers.InSecondaryContext()?1:0;
        gCoOpCameraPolicy.locked=locked;
        gCoOpCameraPolicy.lockPending=1u<<(1-slot);++gCoOpCameraPolicyLocks[slot];
    }
}
// Update flush camera policy in the selected camera's private state.
static void CoOp_FlushCameraPolicy() {
    if(!gCoOpSession.enabled) {gCoOpCameraPolicy=CoOpCameraPolicy();return;}
    unsigned slot=0;
    CoOp_ApplyCameraPolicy(&slot);
    slot=1;
    if((gCoOpCameraPolicy.modePending|gCoOpCameraPolicy.lockPending)&2)
        gCoOpPlayers.WithSecondaryContext(CoOp_ApplyCameraPolicy,&slot);
}
// Install install camera policy hooks at the supported retail call site, preserving its fallback.
static bool CoOp_InstallCameraPolicyHooks() {
    const u32 sites[]={0x004E274C,0x004E23F8};
    const u32 expected[]={0x004167F0,0x004B9E10};
    for(unsigned i=0;i<2;++i) {
        const u8* code=reinterpret_cast<const u8*>(sites[i]);
        if(code[0]!=0xE8 || sites[i]+5+*reinterpret_cast<const i32*>(code+1)!=expected[i])return false;
    }
    DWORD old,unused;
    if(!VirtualProtect(reinterpret_cast<void*>(0x004E2000),0x1000,PAGE_EXECUTE_READWRITE,&old))return false;
    PATCH_CALL(0x004E274C,CoOp_ScriptCameraMode);
    PATCH_CALL(0x004E23F8,CoOp_ScriptCameraLock);
    VirtualProtect(reinterpret_cast<void*>(0x004E2000),0x1000,old,&unused);
    FlushInstructionCache(GetCurrentProcess(),0,0);gCoOpCameraPolicyInstalled=true;return true;
}
// Update camera policy status in the selected camera's private state.
static void CoOp_CameraPolicyStatus(FILE* log) {
    fprintf(log,"camera-policy: installed=%u mode=%d lock=%u pending=%u,%u modes=%u,%u locks=%u,%u\n",
        gCoOpCameraPolicyInstalled,(int)gCoOpCameraPolicy.mode,gCoOpCameraPolicy.locked,
        gCoOpCameraPolicy.modePending,gCoOpCameraPolicy.lockPending,
        gCoOpCameraPolicyModes[0],gCoOpCameraPolicyModes[1],gCoOpCameraPolicyLocks[0],gCoOpCameraPolicyLocks[1]);
}
#endif
