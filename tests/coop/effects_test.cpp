#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "coop/presentation/coop_effects.h"
typedef unsigned char u8;
typedef unsigned long u32;
typedef long i32;
static u8 memoryImage[0x300000];
static u32 Address(u32 a) {assert(a>=0x400000 && a<0x700000);return (u32)(memoryImage+a-0x400000);}
static u32 Retail(u32 a) {return a? a-(u32)memoryImage+0x400000:0;}
#define CELL(T,a) (*reinterpret_cast<T*>(Address(a)))
struct TestSlot {void** list;void (__cdecl *draw)(void**);};
static TestSlot slots[32];
static int age=37,worldCalls,commonCalls,originalCalls,expectedCamera;
static void DrawSlot(void (__cdecl *fn)(void**),void** head) {
    assert(*head==&age && age==37);
    if((u32)fn==0x40DBD0) {assert(CELL(int,0x56F1B4)==expectedCamera);++worldCalls;}
    else ++commonCalls;
}
static void __cdecl Original() {
    ++originalCalls;
    for(int i=0;i<32;++i) if(slots[i].list) DrawSlot(slots[i].draw,slots[i].list);
}
static void __cdecl SetRot(void* p) {assert(p==(void*)Address(0x56F224));}
static void __cdecl Zero() {CELL(int,0x610B34)=0;}
static void __cdecl Rtps() {}
#include "effects_runtime_test.h"
static int projectedX,projectedY,reticles;
static void __fastcall Reticle(void* player,void*) {
    assert(player==&age && CELL(int,0x56F1B4)==200 && gCoOpEffectsDrawing);
    CELL(long,0x610BA0)=256;CELL(long,0x610BA4)=120;CELL(long,0x610BA8)=100;
    CELL(long,0x54F03C)=276;CELL(long,0x54F040)=512;CELL(long,0x54F044)=240;
    CELL(long,0x654F5C)=4096;CELL(long,0x654F58)=4096;
    CoOp_EffectRtps();assert(CELL(long,0x610BA0)==projectedX && CELL(long,0x610BA4)==projectedY);
    CELL(int,0x610B34)=999;++reticles;
}
int main() {
    assert(CoOp_WorldEffectSlot(0x56EB2C,0x4097E0));
    assert(CoOp_WorldEffectSlot(0x56EA50,0x40DBD0));
    assert(!CoOp_WorldEffectSlot(0x56E9DC,0x40D7C0));
    assert(!CoOp_WorldEffectSlot(0x56EA50,0x40D7C0));
    CELL(void*,0x56EB50)=(u8*)slots-8;
    slots[0].list=(void**)Address(0x56EA50);slots[0].draw=(void (__cdecl*)(void**))0x40DBD0;
    slots[1].list=(void**)Address(0x56E9DC);slots[1].draw=(void (__cdecl*)(void**))0x40D7C0;
    *slots[0].list=*slots[1].list=&age;
    CELL(int,0x568154)=1280;CELL(int,0x568158)=720;
    CELL(int,0x61B5FC)=512;CELL(int,0x628614)=240;
    float* proj=(float*)Address(0x56E570);proj[0]=proj[5]=500;proj[8]=960;proj[9]=360;
    unsigned char camera[0x94]={0};*(int*)(camera+4)=200;
    CELL(int,0x56F1B4)=100;CELL(int,0x610B34)=777;
    gCoOpEffectsInstalled=true;expectedCamera=200;
    CoOp_DrawWorldEffects(camera,1);CoOp_DrawWorldEffects(camera,2);
    assert(worldCalls==2 && commonCalls==0 && age==37);
    assert(CELL(int,0x56F1B4)==100 && CELL(int,0x610B34)==777);
    CoOp_DisplayRemainingEffects();
    assert(originalCalls==1 && worldCalls==2 && commonCalls==1);
    assert(slots[0].list==(void**)Address(0x56EA50));
    expectedCamera=100;CoOp_DisplayRemainingEffects();
    assert(originalCalls==2 && worldCalls==3 && commonCalls==2); // ordinary fallback
    CoOpEffectProjection p;assert(p.Set(proj,1280,720,512,240));
    long x=256,y=120;p.Remap(x,y,100,276,512,240,6553,4096);
    assert(x==384 && y==120);
    x=y=32768;p.Remap(x,y,0,276,512,240,6553,4096);assert(x==32768 && y==32768);
    assert(!p.Set(proj,0,720,512,240));
    gCoOpRtpsOriginal=(void*)&Rtps;
    for(int horizontal=0;horizontal<2;++horizontal)for(int owner=0;owner<2;++owner) {
        projectedX=horizontal?256:(owner?384:128);projectedY=horizontal?(owner?180:60):120;
        proj[8]=projectedX*1280.0f/512;proj[9]=projectedY*720.0f/240;
        CoOp_DrawProjectedPlayer(&age,camera,Reticle);
        assert(CELL(int,0x56F1B4)==100 && CELL(int,0x610B34)==777 && !gCoOpEffectsDrawing);
    }
    assert(reticles==4);
    puts("PASS native reticle projection adapter: both players and split orientations, camera/GTE restoration");
    puts("PASS: per-view effects, common text once, ordinary fallback, immutable lifetime, restored camera/GTE/slots, projection centers and zero-depth sentinel");
    return 0;
}
