// Co-op / player: Construct, update, carry, and recover each independent retail CPlayer.
#pragma once
#ifndef COOP_ARMOR_RUNTIME_H
#define COOP_ARMOR_RUNTIME_H
static bool gCoOpArmorRenderInstalled;
static unsigned gCoOpArmorDraws[2];
// Update armor render region on the selected CPlayer's independent lifecycle.
static u32 __cdecl CoOp_ArmorRenderRegion(CSuper* body) {
    // MechList describes the VIEW owner for ceiling fading. Armor belongs to
    // the BODY being drawn and must be visible from either player's camera.
    CPlayer* player=0;unsigned slot=COOP_PLAYER_COUNT;
    if(gCoOpSession.enabled && gCoOpPlayers.Get(COOP_PLAYER_TWO))
        for(unsigned i=0;i<COOP_PLAYER_COUNT;++i)
            if(body==gCoOpPlayers.Get((CoOpPlayerSlot)i)) {
                player=static_cast<CPlayer*>(body);slot=i;break;
            }
    if(!player && body==*reinterpret_cast<CPlayer**>(0x006A9038) &&
       !*reinterpret_cast<i32*>(0x006B78F8))player=static_cast<CPlayer*>(body);
    if(player && player->field_5E9) {
        i32 region=*reinterpret_cast<i32*>(0x0054AE18);
        if(region>=0 && region<40) {
            if(slot<COOP_PLAYER_COUNT)++gCoOpArmorDraws[slot];
            return (u32)region;
        }
    }
    return body->mRegion;
}
// Update armor region bridge on the selected CPlayer's independent lifecycle.
static void __declspec(naked) CoOp_ArmorRegionBridge() {
    // Retail EBP is this CSuper. +B0 is its selected model region; replace
    // only the native view-owner armor test. Leave ceiling/view state intact.
    __asm {
        push ecx
        push edx
        push ebp
        call CoOp_ArmorRenderRegion
        add esp,4
        mov [esp+0B8h],eax
        pop edx
        pop ecx
        push 00474D86h
        ret
    }
}
// Install install armor render hook at the supported retail call site, preserving its fallback.
static bool CoOp_InstallArmorRenderHook() {
    const u32 site=0x00474D5E;
    const u8 expected[]={0x8B,0x44,0x24,0x2C,0x85,0xC0,0x74,0x20,
        0xA1,0xF8,0x78,0x6B,0x00,0x85,0xC0,0x75,0x17,
        0x8A,0x85,0xE9,0x05,0x00,0x00,0x84,0xC0,0x74,0x0D,
        0x8B,0x15,0x18,0xAE,0x54,0x00,0x89,0x94,0x24,0xB0,0x00,0x00,0x00};
    if(memcmp(reinterpret_cast<void*>(site),expected,sizeof(expected)))return false;
    DWORD old,unused;
    if(!VirtualProtect(reinterpret_cast<void*>(site),6,PAGE_EXECUTE_READWRITE,&old))return false;
    *reinterpret_cast<u8*>(site)=0xE9;
    *reinterpret_cast<i32*>(site+1)=reinterpret_cast<u32>(&CoOp_ArmorRegionBridge)-site-5;
    *reinterpret_cast<u8*>(site+5)=0x90;
    VirtualProtect(reinterpret_cast<void*>(site),6,old,&unused);
    FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(site),6);
    gCoOpArmorRenderInstalled=true;return true;
}
struct CoOpArmorContextStatus {unsigned latch;void* portrait;};
// Return read armor context from the selected CPlayer without overwriting the other player's state.
static void CoOp_ReadArmorContext(void* argument) {
    CoOpArmorContextStatus* state=static_cast<CoOpArmorContextStatus*>(argument);
    state->latch=*reinterpret_cast<u8*>(0x006A9040);
    state->portrait=*reinterpret_cast<void**>(0x0060F754);
}
// Update armor status on the selected CPlayer's independent lifecycle.
static void CoOp_ArmorStatus(FILE* file) {
    CoOpArmorContextStatus first,second;memset(&second,0,sizeof(second));
    CoOp_ReadArmorContext(&first);
    gCoOpPlayers.WithSecondaryContext(CoOp_ReadArmorContext,&second);
    CPlayer* p1=gCoOpPlayers.Get(COOP_PLAYER_ONE);CPlayer* p2=gCoOpPlayers.Get(COOP_PLAYER_TWO);
    fprintf(file,"armor: render-hook=%d P1=%u P2=%u latch=%u,%u portrait=%p,%p capacity=%ld,%ld model-draws=%u,%u\n",
        (int)gCoOpArmorRenderInstalled,p1?(unsigned)p1->field_5E9:0,p2?(unsigned)p2->field_5E9:0,
        first.latch,second.latch,first.portrait,second.portrait,
        p1?p1->field_5EC:0,p2?p2->field_5EC:0,gCoOpArmorDraws[0],gCoOpArmorDraws[1]);
}
#endif
