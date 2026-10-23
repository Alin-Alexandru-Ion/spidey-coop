#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef unsigned long u32;typedef unsigned short u16;typedef int i32;
struct Vec {int vx,vy,vz;};
struct CDummy {u32* vtable;int mNumFrames,mAnim,frame;Vec mPos,mAngles;};
static int gCoOpMainSubmenu,created,deleted,ticks,effects,draws;
static int currentSuit=3;
static u32 costumeUnlocks;
static const char* createdModels[8];
static int constructedSuit[8];
static void* Address(u32 address){assert(address==0x006828D8);return &costumeUnlocks;}
#include "coop/player/coop_costume.h"
static CDummy* drawn[2];
static void __fastcall Destroy(CDummy* p,void*,int f){assert(f==1);++deleted;free(p);}
static void __fastcall AI(CDummy* p,void*){++p->frame;++ticks;}
static u32 vtable[]={(u32)&Destroy,0,(u32)&AI};
static void* __cdecl Allocate(u32 size){return calloc(1,size);}
static CDummy* __fastcall Construct(CDummy* p,void*,const char* model,int,int,int,int,u16*,u16*,u16*,int,int,int,int){
    assert(created<8);createdModels[created]=model;constructedSuit[created]=currentSuit;
    ++created;p->vtable=vtable;p->mNumFrames=60;p->mAnim=5;p->frame=0;return p;
}
static void __fastcall RunAnim(CDummy* p,void*,int anim,int from,int to){assert(anim==5 && to==-1);p->frame=from;}
static void __cdecl Draw(void* p){assert(draws<2);drawn[draws++]=(CDummy*)p;}
static void __cdecl Effects(){++effects;}
#include "menu_preview_runtime_test.h"
int main(){
    CDummy first={0};
    gCoOpMainSubmenu=2;
    assert(CoOp_MenuDummyCtor(&first,0,"spidey",50,4096,-32,0,0,0,0,0,0,0,0)==&first);
    assert(created==2 && gCoOpMenuPartner && gCoOpMenuPartner!=&first);
    assert(!strcmp(createdModels[0],"spidey") && !strcmp(createdModels[1],"spReilly"));
    assert(constructedSuit[0]==3 && constructedSuit[1]==8 && currentSuit==3);
    assert(first.frame==0 && gCoOpMenuPartner->frame==20);
    first.mPos.vx=20;first.mPos.vy=-32*4096;first.mPos.vz=408*4096;
    CoOp_MenuDrawPreview(0);assert(draws==2 && drawn[0]==&first && drawn[1]==gCoOpMenuPartner);
    assert(gCoOpMenuPartner->mPos.vz>first.mPos.vz && gCoOpMenuPartner->mPos.vx>first.mPos.vx);
    CoOp_MenuUpdateEffects();assert(first.frame==0 && gCoOpMenuPartner->frame==21 && ticks==1 && effects==1);
    CoOp_MenuDestroyPreview();CoOp_MenuDestroyPreview();assert(deleted==1 && !gCoOpMenuPartner);
    costumeUnlocks=1u<<1;gCoOpSelectedSuit=2;
    CoOp_MenuDummyCtor(&first,0,"spidey",50,4096,-32,0,0,0,0,0,0,0,0);
    assert(created==4 && !strcmp(createdModels[3],"sp2099"));
    assert(constructedSuit[3]==2 && currentSuit==3);
    CoOp_MenuDestroyPreview();assert(deleted==2);
    costumeUnlocks=0;gCoOpSelectedSuit=10;
    CoOp_MenuDummyCtor(&first,0,"spidey",50,4096,-32,0,0,0,0,0,0,0,0);
    assert(created==6 && !strcmp(createdModels[5],"spReilly"));
    assert(constructedSuit[5]==8 && currentSuit==3);
    CoOp_MenuDestroyPreview();assert(deleted==3);
    gCoOpMainSubmenu=1;CoOp_MenuDummyCtor(&first,0,"spidey",50,4096,-32,0,0,0,0,0,0,0,0);
    assert(created==7 && !gCoOpMenuPartner);draws=0;CoOp_MenuDrawPreview(&first);assert(draws==1);
    CoOp_MenuDestroyPreview();assert(deleted==3);
    puts("PASS native menu actors: selected P2 model/default, P1 suit restoration, separate phase/update, ordinary fallback, cleanup once");
    return 0;
}
