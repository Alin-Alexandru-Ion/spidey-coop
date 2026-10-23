#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "coop/core/coop_router.h"
struct CRhino:CSuper {};
static struct {bool enabled;} gCoOpSession={true};
static CPlayer partner;
static struct Registry {bool second;CPlayer* Get(CoOpPlayerSlot) {return second?&partner:0;}} gCoOpPlayers;
static unsigned nativeStarts;
static void __fastcall NativeRun(CRhino* boss,void*,i32 anim,i32 from,i32 to) {
    ++nativeStarts;boss->mAnim=(u16)anim;boss->mNumFrames=12;
    if(to==-1)to=boss->mNumFrames-1;
    boss->mFrame=(short)from;boss->mFrameFrac=0;boss->mTargetFrame=(short)to;
    boss->mAnimMode=0;boss->mAnimDir=to>from?1:(to<from?-1:0);
    boss->mAnimFinished=from==to;
}
static void HalfFrame(CRhino& boss) {
    // Retail UpdateFrame: speed=65536, delta=1 -> 32768 fractional units.
    u32 frame=((u32)boss.mFrame<<16)|(u16)boss.mFrameFrac;
    if(boss.mAnimDir==1)frame+=32768;
    boss.mFrame=(short)(frame>>16);boss.mFrameFrac=(short)frame;
    if(boss.mAnimMode==0 && boss.mFrame>=boss.mTargetFrame) {
        boss.mFrame=boss.mTargetFrame;boss.mAnimFinished=1;
    }
}
#include "rhino_shock_runtime_test.h"
int main() {
    CRhino boss;memset(&boss,0,sizeof(boss));boss.mNumFrames=12;
    boss.mAnim=30;boss.mAnimMode=1;boss.mAnimDir=1;gCoOpPlayers.second=true;
    for(unsigned i=0;i<120;++i) {HalfFrame(boss);NativeRun(&boss,0,30,boss.mFrame,-1);}
    assert(boss.mFrame==0 && !boss.mAnimFinished); // reproduce original stall
    nativeStarts=0;
    for(i=0;i<24;++i) {
        HalfFrame(boss);CoOp_RhinoFinishShockAnim(&boss,0,30,boss.mFrame,-1);
    }
    assert(boss.mAnimFinished && boss.mFrame==11 && nativeStarts==0);
    assert(gCoOpRhinoShockContinues==24);
    boss.mAnimMode=1;boss.mFrame=2;boss.mAnimFinished=0;
    CoOp_RhinoFinishShockAnim(&boss,0,30,boss.mFrame,-1);assert(nativeStarts==1 && boss.mAnimMode==0);
    HalfFrame(boss);short fraction=boss.mFrameFrac;
    CoOp_RhinoFinishShockAnim(&boss,0,30,boss.mFrame,-1);
    assert(boss.mFrameFrac==fraction && nativeStarts==1);
    gCoOpSession.enabled=false;
    CoOp_RhinoFinishShockAnim(&boss,0,30,boss.mFrame,-1);assert(nativeStarts==2 && !boss.mFrameFrac);
    gCoOpSession.enabled=true;gCoOpPlayers.second=false;
    CoOp_RhinoFinishShockAnim(&boss,0,30,boss.mFrame,-1);assert(nativeStarts==3);
    puts("PASS Rhino shock: reproduces native fractional-frame stall, finishes through native animation, isolated co-op transition, singleplayer unchanged");
    return 0;
}
