// Co-op / player: Construct, update, carry, and recover each independent retail CPlayer.
#pragma once
#include "spidey.h"
// World Y increases downward. Use the retail direction helper (forward is
// negative local Z), with negative magnitude to obtain directly behind.
static unsigned gCoOpLastSpawnStrategy;
// Return find spawn from the selected CPlayer without overwriting the other player's state.
static bool CoOp_FindSpawn(CPlayer* primary,CVector& result,bool chaseLedgeFallback=false) {
    typedef i32 (__cdecl *FloorFn)(CVector*,i32,i32,CBody**);
    typedef i32 (__cdecl *ClearFn)(CVector*,CVector*,CVector*,i32);
    FloorFn floor=reinterpret_cast<FloorFn>(0x004E6840);
    ClearFn clear=reinterpret_cast<ClearFn>(0x004E67A0);
    gCoOpLastSpawnStrategy=0;
    CVector origin=primary->mPos;
    int i;
    if(chaseLedgeFallback) {
        // Chase restarts can place P1 on a wall, a ledge edge or in scripted
        // motion. Start from the exact position retail gave P1. Only separate
        // P2 sideways when a nearby destination has support and clear space.
        const short sides[]={1024,-1024};
        const int distances[]={16,24,32,48,64};
        const int footprint[4][2]={{8,0},{-8,0},{0,8},{0,-8}};
        for(unsigned side=0;side<2;++side)for(unsigned step=0;step<5;++step) {
            short angles[3]={0,(short)((primary->mAngles.vy+sides[side])&4095),0};
            CVector offset,spot=origin;
            reinterpret_cast<void (__cdecl*)(CVector*,int,short*)>(0x004E60C0)(
                &offset,-distances[step],angles);
            spot.vx+=offset.vx;spot.vz+=offset.vz;
            CVector ray=spot;ray.vy-=256*4096;
            i32 ground=floor(&ray,16,512,0);
            if(ground==-1 || ground<origin.vy-256*4096 ||
                ground>origin.vy+160*4096)continue;
            bool safe=true;
            for(i=0;i<4 && safe;++i) {
                CVector probe=ray;probe.vx+=footprint[i][0]*4096;
                probe.vz+=footprint[i][1]*4096;
                i32 y=floor(&probe,16,512,0);
                safe=y!=-1 && y>=ground-16*4096 && y<=ground+16*4096;
            }
            spot.vy=ground-12*4096;
            CVector start=origin;start.vy-=12*4096;
            for(i=0;i<3 && safe;++i) {
                CVector a=start,b=spot;a.vy-=i*32*4096;b.vy-=i*32*4096;
                safe=clear(&a,&b,0,0)!=0;
                for(int j=0;j<4 && safe;++j) {
                    CVector edge=b;edge.vx+=footprint[j][0]*4096;
                    edge.vz+=footprint[j][1]*4096;
                    safe=clear(&b,&edge,0,0)!=0;
                }
            }
            CVector head=spot;head.vy-=64*4096;
            if(safe && clear(&spot,&head,0,0)) {
                result=spot;gCoOpLastSpawnStrategy=2;return true;
            }
        }
        // P1's authored position remains the only known usable position when
        // no nearby floor is reported (notably wall-running chase entries).
        // Join there and inherit its current velocity; gameplay can separate
        // the two bodies once a supported side is available.
        result=origin;gCoOpLastSpawnStrategy=3;return true;
    }
    i32 base=floor(&origin,16,256,0);
    bool supported=base!=-1 && base>=origin.vy-16*4096 &&
        base<=origin.vy+160*4096;
    if(!supported)return false;
    short angles[3]={0,primary->mAngles.vy,0};
    const int distances[]={64,48,32,16};
    for(unsigned n=0;n<4;++n) {
        int distance=distances[n];
        CVector offset,spot;
        reinterpret_cast<void (__cdecl*)(CVector*,int,short*)>(0x004E60C0)(&offset,-distance,angles);
        spot=origin;spot.vx+=offset.vx;spot.vz+=offset.vz;
        CVector ray=spot;
        i32 ground=floor(&ray,16,256,0);
        if(ground==-1 || ground<base-24*4096 || ground>base+24*4096)continue;
        bool safe=true;
        // Require supporting floor across a small footprint, not just one ray
        // on the edge of a roof. These offsets probe support, not spawn positions.
        const int footprint[4][2]={{16,0},{-16,0},{0,16},{0,-16}};
        for(i=0;i<4 && safe;++i) {
            CVector probe=ray;probe.vx+=footprint[i][0]*4096;probe.vz+=footprint[i][1]*4096;
            i32 y=floor(&probe,16,256,0);
            safe=y!=-1 && y>=ground-24*4096 && y<=ground+24*4096;
        }
        if(!safe) continue;
        spot.vy=origin.vy+(ground-base)-12*4096;
        // Sweep the short move from P1 and check clearance up the body and
        // around its destination. Retain native world collision filtering.
        CVector start=origin;start.vy-=12*4096;
        for(i=0;i<3 && safe;++i) {
            CVector a=start,b=spot;a.vy-=i*32*4096;b.vy-=i*32*4096;
            safe=clear(&a,&b,0,0)!=0;
            for(int j=0;j<4 && safe;++j) {
                CVector edge=b;edge.vx+=footprint[j][0]*4096;edge.vz+=footprint[j][1]*4096;
                safe=clear(&b,&edge,0,0)!=0;
            }
        }
        CVector head=spot;head.vy-=64*4096;
        if(safe && clear(&spot,&head,0,0)) {
            result=spot;gCoOpLastSpawnStrategy=1;return true;
        }
    }
    return false;
}
struct CoOpSpawnGate {
    bool armed;unsigned long since;
    // Initialize co-op spawn gate with no active retail ownership.
    CoOpSpawnGate():armed(false),since(0) {}
    // Release owned resources and reset the selected CPlayer without overwriting the other player's state to its inactive state.
    void Reset(){armed=false;}
    // Update ready on the selected CPlayer's independent lifecycle.
    bool Ready(bool blocked,unsigned long now) {
        if(blocked){Reset();return false;}
        if(!armed){armed=true;since=now;return false;}
        return (unsigned long)(now-since)>=350;
    }
};
// Prepare spawn camera mode ready for the selected CPlayer without overwriting the other player's state.
inline bool CoOp_SpawnCameraModeReady(i32 mode) {return mode==3 || mode==16;}
// Retail synthesized input owns P1 during authored chase traversal. Its
// SwitchToSynthesizedInput entry sets field_1AC to 1 and the native player
// update clears it at the handoff back to normal controls.
inline bool CoOp_PrimaryControlReady(const CPlayer* player) {
    return player && !player->field_1AC;
}
// Return find entry spawn from the selected CPlayer without overwriting the other player's state.
static bool CoOp_FindEntrySpawn(CPlayer* actor,CVector& result) {
    // A floor-only rear placement is valid for an upright ground state. An
    // attached or airborne CPlayer must keep its native position/normal/rope
    // instead of being moved down onto an unrelated floor. Exact overlap is
    // usable by both controllers and does not postpone control until landing.
    const u8* body=reinterpret_cast<const u8*>(actor);
    u32 state=*reinterpret_cast<const u32*>(body+0xE1C);
    if((actor->mCollision&2) && actor->field_A8.vy==-4096 &&
       !(state&~(1u|8u|0x400000u)) && !body[0xAD4] &&
       !*reinterpret_cast<void* const*>(body+0xE64) && CoOp_FindSpawn(actor,result))return true;
    result=actor->mPos;gCoOpLastSpawnStrategy=4;return true;
}
// Report whether spawn blocked for the selected CPlayer without overwriting the other player's state.
static bool CoOp_SpawnBlocked(bool shadowStart=false) {
    CPlayer* primary=*reinterpret_cast<CPlayer**>(0x006A9038);
    void* camera=*reinterpret_cast<void**>(0x0056F3B8);
    if(!primary || !camera || *reinterpret_cast<i32*>(0x00ADB4D8))return true;
    // A shadow participant is constructed during the authored opening and
    // stays hidden. Only its later visible/control release requires a normal
    // native route and letterbox to finish. Camera mode and a floor beneath
    // P1 do not define whether a crawling/swinging player has control.
    return !shadowStart && (!CoOp_PrimaryControlReady(primary) ||
        *reinterpret_cast<i32*>(0x00660F80));
}
