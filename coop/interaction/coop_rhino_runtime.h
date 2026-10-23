// Co-op / interaction: Adapt native enemy, projectile, boss, and object interactions.
#pragma once
#ifndef COOP_RHINO_RUNTIME_H
#define COOP_RHINO_RUNTIME_H
static bool gCoOpRhinoShockInstalled;
static unsigned gCoOpRhinoShockStarts,gCoOpRhinoShockContinues;
static void __fastcall CoOp_RhinoFinishShockAnim(CRhino* boss,void*,i32 anim,i32 from,i32 to) {
    // GetShocked at 0047E4DE calls RunAnim(30,currentFrame,-1) EVERY tick
    // after the shock timer expires. RunAnim clears mFrameFrac, so a native
    // half-frame update never reaches the next frame. Change only the cycle
    // -> one-shot transition; keep native timer, HP interpolation and recovery.
    const i32 end=to==-1 ? boss->mNumFrames-1:to;
    if(gCoOpSession.enabled && gCoOpPlayers.Get(COOP_PLAYER_TWO) &&
       boss->mAnim==anim && boss->mAnimMode==0 && boss->mTargetFrame==end &&
       (boss->mAnimDir==1 || boss->mAnimFinished)) {
        ++gCoOpRhinoShockContinues;return;
    }
    ++gCoOpRhinoShockStarts;
    reinterpret_cast<void (__fastcall*)(CRhino*,void*,i32,i32,i32)>(0x00460C10)(
        boss,0,anim,from,to);
}
// Install install rhino shock hook at the supported retail call site, preserving its fallback.
static bool CoOp_InstallRhinoShockHook() {
    const u32 site=0x0047E4DE;
    if(*reinterpret_cast<u8*>(site)!=0xE8 ||
       site+5+*reinterpret_cast<i32*>(site+1)!=0x00460C10)return false;
    DWORD old,unused;
    if(!VirtualProtect(reinterpret_cast<void*>(site),5,PAGE_EXECUTE_READWRITE,&old))return false;
    PATCH_CALL(site,CoOp_RhinoFinishShockAnim);
    VirtualProtect(reinterpret_cast<void*>(site),5,old,&unused);
    FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(site),5);
    gCoOpRhinoShockInstalled=true;return true;
}
// Route rhino status through the interacting player's native context.
static void CoOp_RhinoStatus(FILE* file) {
    fprintf(file,"rhino-shock: installed=%d starts=%u continued=%u\n",
        (int)gCoOpRhinoShockInstalled,gCoOpRhinoShockStarts,gCoOpRhinoShockContinues);
    for(CItem* item=*reinterpret_cast<CItem**>(0x0056E990);item;item=item->mNextItem) {
        if(item->mType!=307)continue;
        CRhino* boss=static_cast<CRhino*>(item);const u8* raw=reinterpret_cast<const u8*>(boss);
        fprintf(file,"rhino-state: ptr=%p health=%d phase=%lu substate=%ld shock-timer=%ld base-hp=%d animation=%u frame=%d fraction=%d target=%d mode=%u direction=%d finished=%u speed=%ld delta=%ld flags=%04x body-flags=%04x\n",
            boss,boss->mHealth,(u32)boss->field_31C.bothFlags,boss->dumbAssPad,
            *reinterpret_cast<const i32*>(raw+0x34C),*reinterpret_cast<const i16*>(raw+0x354),
            (unsigned)boss->mAnim,(int)boss->mFrame,(int)boss->mFrameFrac,
            (int)boss->mTargetFrame,(unsigned)boss->mAnimMode,(int)boss->mAnimDir,
            (unsigned)boss->mAnimFinished,boss->mAnimSpeed,boss->field_80,
            boss->mFlags,boss->mCBodyFlags);
    }
}
#endif
