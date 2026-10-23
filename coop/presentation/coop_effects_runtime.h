// Co-op / presentation: Render co-op HUD, effects, display modes, and capture diagnostics.
#pragma once
#include "coop_effects.h"
struct CoOpRetailEffectSlot {void** list;void (__cdecl *draw)(void**);};
static bool gCoOpEffectsInstalled,gCoOpEffectsDrawing,gCoOpEffectsHandled;
static void* gCoOpEffectsCamera;
static unsigned gCoOpEffectsCalls[2],gCoOpEffectsUnknown;
static CoOpEffectProjection gCoOpEffectProjection;
static void* gCoOpRtpsOriginal;
// Prepare effect rtps for the owning viewport.
static void __cdecl CoOp_EffectRtps() {
    reinterpret_cast<void (__cdecl*)()>(gCoOpRtpsOriginal)();
    if(gCoOpEffectsDrawing) gCoOpEffectProjection.Remap(
        *reinterpret_cast<long*>(0x00610BA0),*reinterpret_cast<long*>(0x00610BA4),
        *reinterpret_cast<long*>(0x00610BA8),*reinterpret_cast<long*>(0x0054F03C),
        *reinterpret_cast<long*>(0x0054F040),*reinterpret_cast<long*>(0x0054F044),
        *reinterpret_cast<long*>(0x00654F5C),*reinterpret_cast<long*>(0x00654F58));
}
// Prepare effect slots for the owning viewport.
static CoOpRetailEffectSlot* CoOp_EffectSlots() {
    u8* server=*reinterpret_cast<u8**>(0x0056EB50);
    return server?reinterpret_cast<CoOpRetailEffectSlot*>(server+8):0;
}
// Reticles use the same legacy RTPS projection as world effects. Apply the
// active pane's focal length/center while keeping original targeting and art.
static void CoOp_DrawProjectedPlayer(void* player,void* camera,void (__fastcall *draw)(void*,void*)) {
    if(!gCoOpEffectsInstalled || !camera){draw(player,0);return;}
    CoOpEffectProjection savedProjection=gCoOpEffectProjection;
    if(!gCoOpEffectProjection.Set(reinterpret_cast<float*>(0x0056E570),
        *reinterpret_cast<int*>(0x00568154),*reinterpret_cast<int*>(0x00568158),
        *reinterpret_cast<int*>(0x0061B5FC),*reinterpret_cast<int*>(0x00628614))) {draw(player,0);return;}
    u8 savedCamera[0x94],savedGte[0x100],savedMatrices[0x248];
    memcpy(savedCamera,reinterpret_cast<void*>(0x0056F1B0),sizeof(savedCamera));
    memcpy(savedGte,reinterpret_cast<void*>(0x00610B20),sizeof(savedGte));
    memcpy(savedMatrices,reinterpret_cast<void*>(0x0056E570),sizeof(savedMatrices));
    memcpy(reinterpret_cast<void*>(0x0056F1B0),camera,sizeof(savedCamera));
    bool savedDrawing=gCoOpEffectsDrawing;gCoOpEffectsDrawing=true;
    draw(player,0);
    gCoOpEffectsDrawing=savedDrawing;gCoOpEffectProjection=savedProjection;
    memcpy(reinterpret_cast<void*>(0x0056F1B0),savedCamera,sizeof(savedCamera));
    memcpy(reinterpret_cast<void*>(0x00610B20),savedGte,sizeof(savedGte));
    memcpy(reinterpret_cast<void*>(0x0056E570),savedMatrices,sizeof(savedMatrices));
}
// Draw draw world effects in the owning player's viewport.
static void CoOp_DrawWorldEffects(void* camera,unsigned owner) {
    CoOpRetailEffectSlot* slots=CoOp_EffectSlots();
    if(!gCoOpEffectsInstalled || !camera || !slots || owner<1 || owner>2) return;
    if(!gCoOpEffectProjection.Set(reinterpret_cast<float*>(0x0056E570),
        *reinterpret_cast<int*>(0x00568154),*reinterpret_cast<int*>(0x00568158),
        *reinterpret_cast<int*>(0x0061B5FC),*reinterpret_cast<int*>(0x00628614))) return;
    u8 savedCamera[0x94],savedGte[0x100],savedMatrices[0x248];
    memcpy(savedCamera,reinterpret_cast<void*>(0x0056F1B0),sizeof(savedCamera));
    memcpy(savedGte,reinterpret_cast<void*>(0x00610B20),sizeof(savedGte));
    memcpy(savedMatrices,reinterpret_cast<void*>(0x0056E570),sizeof(savedMatrices));
    // Effects read MikeCamera directly, including its derived inverse matrix.
    // RenderSetup has populated that matrix in the selected view's scratch.
    memcpy(reinterpret_cast<void*>(0x0056F1B0),camera,sizeof(savedCamera));
    reinterpret_cast<void (__cdecl*)(void*)>(0x0046D7B0)(reinterpret_cast<void*>(0x0056F224));
    reinterpret_cast<void (__cdecl*)()>(0x0046E460)();
    gCoOpEffectsDrawing=true;
    for(unsigned i=0;i<32;++i) {
        if(!slots[i].list) continue;
        if(CoOp_WorldEffectSlot(reinterpret_cast<u32>(slots[i].list),reinterpret_cast<u32>(slots[i].draw))) {
            if(*slots[i].list) {slots[i].draw(slots[i].list);++gCoOpEffectsCalls[owner-1];}
        } else if(owner==1 && reinterpret_cast<u32>(slots[i].list)!=0x0056E9DC) ++gCoOpEffectsUnknown;
    }
    gCoOpEffectsDrawing=false;
    memcpy(reinterpret_cast<void*>(0x0056F1B0),savedCamera,sizeof(savedCamera));
    memcpy(reinterpret_cast<void*>(0x00610B20),savedGte,sizeof(savedGte));
    memcpy(reinterpret_cast<void*>(0x0056E570),savedMatrices,sizeof(savedMatrices));
    if(owner==1) gCoOpEffectsHandled=true;
}
// Prepare display remaining effects for the owning viewport.
static void __cdecl CoOp_DisplayRemainingEffects() {
    typedef void (__cdecl *Fn)();Fn original=reinterpret_cast<Fn>(0x00411CF0);
    CoOpRetailEffectSlot* slots=CoOp_EffectSlots();
    if(!gCoOpEffectsHandled || !slots) {original();return;}
    // Leave common screen text/custom slots at the original frame stage.
    // Suppress the world lists already rendered, without detaching any effects.
    void** saved[32];
    for(unsigned i=0;i<32;++i) {
        saved[i]=slots[i].list;
        if(CoOp_WorldEffectSlot(reinterpret_cast<u32>(slots[i].list),reinterpret_cast<u32>(slots[i].draw))) slots[i].list=0;
    }
    original();
    for(i=0;i<32;++i) slots[i].list=saved[i];
    gCoOpEffectsHandled=false;
}
// Install install effects hooks at the supported retail call site, preserving its fallback.
static bool CoOp_InstallEffectsHooks() {
    const u8 expected[7]={0x0F,0xBF,0x05,0x24,0x0B,0x61,0x00};
    const u32 site=0x00455988;
    if(memcmp(reinterpret_cast<void*>(0x0046DBC0),expected,7) ||
        *reinterpret_cast<u8*>(site)!=0xE8 || site+5+*reinterpret_cast<i32*>(site+1)!=0x00411CF0) return false;
    u8* trampoline=static_cast<u8*>(VirtualAlloc(0,13,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE));
    if(!trampoline) return false;
    memcpy(trampoline,expected,7);trampoline[7]=0x68;
    *reinterpret_cast<u32*>(trampoline+8)=0x0046DBC7;trampoline[12]=0xC3;
    DWORD old,unused;
    if(!VirtualProtect(reinterpret_cast<void*>(0x00401000),0x139000,PAGE_EXECUTE_READWRITE,&old)) {
        VirtualFree(trampoline,0,MEM_RELEASE);return false;
    }
    gCoOpRtpsOriginal=trampoline;
    *reinterpret_cast<i32*>(site+1)=reinterpret_cast<u32>(&CoOp_DisplayRemainingEffects)-site-5;
    *reinterpret_cast<u8*>(0x0046DBC0)=0x68;
    *reinterpret_cast<u32*>(0x0046DBC1)=reinterpret_cast<u32>(&CoOp_EffectRtps);
    *reinterpret_cast<u8*>(0x0046DBC5)=0xC3;*reinterpret_cast<u8*>(0x0046DBC6)=0x90;
    VirtualProtect(reinterpret_cast<void*>(0x00401000),0x139000,old,&unused);
    FlushInstructionCache(GetCurrentProcess(),0,0);
    gCoOpEffectsInstalled=true;return true;
}
