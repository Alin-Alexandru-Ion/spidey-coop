#include "router_test_types.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static i32 clockNow=500;
static u32 Address(u32 a){assert(a==0x006B4CA8);return (u32)&clockNow;}
static int failAt,allocations,frees,animations,starts,swings;
static void* __cdecl Allocate(u32 size){
    ++allocations;if(failAt==allocations)return 0;return calloc(1,size);
}
static void __cdecl EntryFree(void* p){++frees;free(p);}
static void FASTCALL EntryAnim(CPlayer* p,int,i32 anim,i32 frame,i32 target){
    ++animations;p->mAnim=(u16)anim;p->mFrame=(short)frame;
    p->mTargetFrame=(short)target;
}
static void FASTCALL Start(CCamera*,int){++starts;}
static void* FASTCALL DeleteSwing(void* p,int,u32 flags){
    assert(flags==1);EntryFree(*(void**)((u8*)p+0x17C));EntryFree(p);return 0;
}
static u32 swingVtable[]={(u32)&DeleteSwing};
static void* FASTCALL EntrySwing(void* p,int,const void* anchor,i32 length,const void* basis,const void* velocity){
    assert(anchor && basis && velocity && length==200*4096);++swings;
    *(u32**)p=swingVtable;
    *(void**)((u8*)p+0x17C)=calloc(1,120);return p;
}
#include "coop/player/coop_entry_state.h"
static u32& Word(CPlayer& p,unsigned offset){return *(u32*)((u8*)&p+offset);}
int main(){
    assert(sizeof(CSuper)==0x1A4 && sizeof(CPlayer)==0xEFC);
    assert(offsetof(CPlayer,field_E0C)==0xE0C && offsetof(CPlayer,mHealth)==0xE2);
    CPlayer actor,partner;memset(&actor,0,sizeof(actor));memset(&partner,0,sizeof(partner));
    actor.mHealth=75;actor.mWebbing=500;actor.field_5D8=1;actor.field_5EC=2;
    partner.mHealth=60;partner.mWebbing=1700;partner.field_5D8=6;partner.field_5EC=3;
    partner.mRegion=8;partner.mModel=12;partner.field_E0C=0xCAFED00D;
    partner.mpPoseBuffer=(void*)0x1234;partner.mpJoints=(void*)0x2345;
    Word(partner,0x1BC)=0x3456;Word(partner,0xE48)=0x4567;
    Word(partner,0xE70)=0x5678;
    actor.mAnim=227;actor.mFrame=17;actor.mFrameFrac=123;
    actor.mAnimMode=1;actor.mAnimDir=-1;actor.mAnimSpeed=0x10000;
    actor.mPos.vx=10*4096;actor.mPos.vy=-500*4096;
    actor.mVel.vx=234;actor.mVel.vy=-56;actor.mAcc.vy=12;
    actor.field_A8.vy=4096;((u8*)&actor)[0xAD4]=1;
    ((u8*)&actor)[0x553]=1;((u8*)&actor)[0x53C]=1;
    Word(actor,0xE1C)=1;Word(actor,0xB8C)=0xABCDEF; // borrowed world collision face
    actor.field_1AC=1;
    CoOp_ShadowEntry(&partner,&actor);
    assert(partner.mFrame==17 && partner.mAnim==227 && partner.mFrameFrac==123);
    CVector exit=actor.mPos;exit.vx+=32*4096;
    assert(!CoOp_AdoptEntryMotion(&partner,&actor,exit));
    actor.field_1AC=0;
    // A restrained actor must not give its state to an unrestrained partner.
    Word(actor,0xE1C)=0x10000000;u32 ownState=Word(partner,0xE1C);
    assert(!CoOp_AdoptEntryMotion(&partner,&actor,exit) && Word(partner,0xE1C)==ownState);
    Word(actor,0xE1C)=0x20000000;
    assert(!CoOp_AdoptEntryMotion(&partner,&actor,exit) && Word(partner,0xE1C)==ownState);
    actor.mHealth=0;Word(actor,0xE1C)=1;
    assert(!CoOp_AdoptEntryMotion(&partner,&actor,exit));actor.mHealth=75;
    // Native stand/crawl, jump, fall, swing wind-up, pendulum and zip states.
    const u32 states[]={1,8,2,4,0x100,0x200,0x400,0x1000,0x2000};
    for(unsigned i=0;i<sizeof(states)/sizeof(states[0]);++i){
        Word(actor,0xE1C)=states[i];
        assert(CoOp_AdoptEntryMotion(&partner,&actor,exit));
        assert(Word(partner,0xE1C)==states[i] && partner.mPos.vx==exit.vx);
        assert(partner.mVel.vx==234 && partner.mVel.vy==-56 && partner.mAcc.vy==12);
        assert(partner.field_A8.vy==4096 && ((u8*)&partner)[0xAD4] &&
            ((u8*)&partner)[0x553] && ((u8*)&partner)[0x53C]);
        assert(Word(partner,0xB8C)==0xABCDEF && partner.field_7C==clockNow);
        assert(partner.mFrame==17 && partner.mFrameFrac==123 && partner.mAnimSpeed==0x10000);
        assert(partner.mHealth==60 && partner.mWebbing==1700 && partner.field_5D8==6 && partner.field_5EC==3);
        assert(partner.mRegion==8 && partner.mModel==12 && partner.field_E0C==0xCAFED00D);
        assert(partner.mpPoseBuffer==(void*)0x1234 && partner.mpJoints==(void*)0x2345);
        assert(Word(partner,0x1BC)==0x3456 && Word(partner,0xE48)==0x4567 && Word(partner,0xE70)==0x5678);
    }
    u8 path[0x180],rope[0x190];memset(path,0x5A,sizeof(path));memset(rope,0,sizeof(rope));
    Word(actor,0xC90)=(u32)path;Word(actor,0xE64)=(u32)rope;Word(actor,0xD7C)=200*4096;
    *(u32*)(rope+0x180)=0x321;*(u32*)(rope+0x108)=200*4096;
    *(u32*)(rope+0x17C)=0x7654;
    failAt=allocations+1;
    assert(!CoOp_AdoptEntryMotion(&partner,&actor,exit) && !Word(partner,0xC90));
    failAt=allocations+2;int freedBefore=frees;
    assert(!CoOp_AdoptEntryMotion(&partner,&actor,exit) && frees==freedBefore+1);
    failAt=0;assert(CoOp_AdoptEntryMotion(&partner,&actor,exit));
    assert(Word(partner,0xC90)!=(u32)path && !memcmp((void*)Word(partner,0xC90),path,sizeof(path)));
    u8* ownRope=(u8*)Word(partner,0xE64);
    assert(ownRope!=rope && *(u32*)(ownRope+0x17C)!=0x7654 && *(u32*)(ownRope+0x180)==0x321);
    ((u8*)Word(partner,0xC90))[0]=0;assert(path[0]==0x5A);
    // Repeated handoff destroys only the recipient's old owned allocations.
    freedBefore=frees;assert(CoOp_AdoptEntryMotion(&partner,&actor,exit));
    assert(frees==freedBefore+3 && Word(actor,0xC90)==(u32)path && Word(actor,0xE64)==(u32)rope);
    Word(actor,0xC90)=Word(actor,0xE64)=0;
    assert(CoOp_AdoptEntryMotion(&partner,&actor,exit));
    CCamera ownerCam,partnerCam;memset(&ownerCam,0x55,sizeof(ownerCam));memset(&partnerCam,0,sizeof(partnerCam));
    partnerCam.mTripod=&partner;partnerCam.vtable=(u32*)0x6789;
    *(u32*)((u8*)&partnerCam+0x13C)=0x789A;
    *(u32*)((u8*)&ownerCam+0x2A0)=16;
    CoOp_AdoptEntryCamera(&partnerCam,&ownerCam);
    assert(partnerCam.mTripod==&partner && partnerCam.vtable==(u32*)0x6789 && starts==1);
    assert(*(u32*)((u8*)&partnerCam+0x13C)==0x789A && *(u32*)((u8*)&partnerCam+0x2A0)==16);
    assert(animations==1 && swings==2);
    puts("PASS native entry transfer: motion/surface/frame continuity, private inventory/input/model/commands, separate rope/path ownership and allocation rollback");
    return 0;
}
