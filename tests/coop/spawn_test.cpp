#include "router_test_types.h"
#include <assert.h>
#include <stdio.h>
#include <math.h>
static int scenario;
static i32 __cdecl SpawnFloor(CVector* p,i32 above,i32 below,CBody**){
    assert(above==16 && (below==256 || below==512));
    if(scenario>=5) {
        // A chase bank begins over open air. The only supported surface is a
        // nearby ledge above and to the side of the airborne primary.
        return scenario!=6 && p->vy<0 && p->vx>=16*4096 ? -128*4096:-1;
    }
    if(scenario==1 && p->vz>48*4096)return -1; // rear ledge; shorter rear distance works
    if(scenario==2)return -1;
    if(scenario==3 && p->vx!=0)return -1; // a center ray alone is insufficient
    return 0;
}
static i32 __cdecl SpawnClear(CVector*,CVector* b,CVector*,i32 mode){
    assert(mode==0);return scenario==7?0:(scenario!=4 || b->vz<=32*4096);
}
static void __cdecl SpawnDirection(CVector* out,int mag,short* angles){
    // Retail direction convention, confirmed at 4E60C0: -sin(yaw), -cos(yaw).
    double yaw=angles[1]*6.283185307179586/4096.0;
    out->vx=(i32)(-mag*4096*sin(yaw));out->vy=0;out->vz=(i32)(-mag*4096*cos(yaw));
}
static bool SpawnBlocked(){return false;}
#include "coop/player/coop_spawn.h"
static CPlayer* spawnPrimary;
static void* spawnCamera;
static int paused,letterbox;
#include "spawn_gate_runtime_test.h"
int main(){
    CPlayer p;memset(&p,0,sizeof(p));CVector out;
    u8 cam[0x300];memset(cam,0,sizeof(cam));spawnPrimary=&p;spawnCamera=cam;
    const i32 modes[]={3,5,16,17};
    for(unsigned mode=0;mode<4;++mode){
        *(i32*)(cam+0x2A0)=modes[mode];
        assert(!CoOp_ActualSpawnBlocked()); // camera style cannot require standing
    }
    p.field_1AC=1;assert(CoOp_ActualSpawnBlocked() && !CoOp_ActualSpawnBlocked(true));
    p.field_1AC=0;letterbox=30;
    assert(CoOp_ActualSpawnBlocked() && !CoOp_ActualSpawnBlocked(true));
    letterbox=0;paused=1;assert(CoOp_ActualSpawnBlocked() && CoOp_ActualSpawnBlocked(true));
    paused=0;spawnPrimary=0;assert(CoOp_ActualSpawnBlocked(true));spawnPrimary=&p;
    assert(CoOp_PrimaryControlReady(&p) && !CoOp_PrimaryControlReady(0));
    p.field_1AC=1;assert(!CoOp_PrimaryControlReady(&p));
    p.field_1AC=0;assert(CoOp_PrimaryControlReady(&p));
    assert(CoOp_SpawnCameraModeReady(3) && CoOp_SpawnCameraModeReady(16));
    assert(!CoOp_SpawnCameraModeReady(5) && !CoOp_SpawnCameraModeReady(17));
    for(int angle=0;angle<4096;angle+=1024){
        p.mAngles.vy=angle;assert(CoOp_FindSpawn(&p,out));
        assert(out.vy==-12*4096);
        double yaw=angle*6.283185307179586/4096.0;
        assert(fabs(out.vx-64*4096*sin(yaw))<2 && fabs(out.vz-64*4096*cos(yaw))<2);
    }
    p.mAngles.vy=0;scenario=1;assert(CoOp_FindSpawn(&p,out) && out.vz==32*4096);
    scenario=2;assert(!CoOp_FindSpawn(&p,out));
    scenario=3;assert(!CoOp_FindSpawn(&p,out));
    scenario=4;assert(CoOp_FindSpawn(&p,out) && out.vz==16*4096);
    scenario=5;assert(!CoOp_FindSpawn(&p,out));
    assert(CoOp_FindSpawn(&p,out,true) && out.vx==24*4096 &&
        out.vy==-140*4096 && gCoOpLastSpawnStrategy==2);
    scenario=6;assert(CoOp_FindSpawn(&p,out,true) && out.vx==p.mPos.vx &&
        out.vy==p.mPos.vy && gCoOpLastSpawnStrategy==3); // exact authored spawn
    scenario=7;assert(CoOp_FindSpawn(&p,out,true) && out.vx==p.mPos.vx &&
        out.vy==p.mPos.vy && gCoOpLastSpawnStrategy==3); // no unsafe separation
    scenario=2;const u32 states[]={1,2,4,0x100,0x200,0x400,0x1000};
    for(unsigned s=0;s<7;++s){
        *(u32*)((u8*)&p+0xE1C)=states[s];
        assert(CoOp_FindEntrySpawn(&p,out) && gCoOpLastSpawnStrategy==4 &&
            out.vx==p.mPos.vx && out.vy==p.mPos.vy && out.vz==p.mPos.vz);
    }
    scenario=0;*(u32*)((u8*)&p+0xE1C)=1;((u8*)&p)[0xAD4]=1;
    assert(CoOp_FindEntrySpawn(&p,out) && gCoOpLastSpawnStrategy==4); // ceiling, floor far below
    ((u8*)&p)[0xAD4]=0;p.mCollision=2;p.field_A8.vy=-4096;assert(CoOp_FindEntrySpawn(&p,out) && gCoOpLastSpawnStrategy==1);
    scenario=0;
    CoOpSpawnGate gate;
    assert(!gate.Ready(true,1000));assert(!gate.Ready(false,2000));
    assert(!gate.Ready(false,2349));assert(gate.Ready(false,2350));
    assert(!gate.Ready(true,2351));assert(!gate.Ready(false,2600));
    assert(gate.Ready(false,2950));gate.Reset();
    assert(!gate.Ready(false,0xFFFFFF00));assert(gate.Ready(false,100));
    puts("PASS entry placement: native script/pause gate, all camera styles, no-floor/ceiling/airborne handoff, supported rear placement and bounded retry timer");
    return 0;
}
