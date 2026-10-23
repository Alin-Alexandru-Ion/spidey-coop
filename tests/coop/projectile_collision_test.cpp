#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "coop/core/coop_router.h"
static CPlayer p1,p2;static bool hidden[2];
static struct {bool enabled;} gCoOpSession={true};
static struct Registry {
    CoOpPlayerRouter router;
    CoOpPlayerRouter& Router(){return router;}
    CPlayer* Get(CoOpPlayerSlot slot){const CoOpPlayerRouter::Participant* p=router.Resolve(router.Handle(slot));return p?p->player:0;}
} gCoOpPlayers;
static bool CoOp_EnemyAlive(CoOpPlayerHandle h,const CPlayer* p,void*) {return p->mHealth>0 && !hidden[h.slot];}
static unsigned queries[2],otherCalls;static bool contact[2];static i32 hitX[2];
static u32 projectionScratch;static i32 bodyScratch;static u8 mathScratch[0x100];
static i32 __cdecl Sphere(CVector*,CVector*,CVector* hit,CBody* list,CBody* ignored,i32 radius) {
    assert(radius==4096);
    if(list!=&p1 && list!=&p2){++otherCalls;return 123;}
    unsigned slot=list==&p2;++queries[slot];projectionScratch=slot+50;
    memset(mathScratch,slot+1,sizeof(mathScratch));bodyScratch=0;
    if(!contact[slot] || list==ignored)return 0;
    hit->vx=hitX[slot];hit->vy=hit->vz=0;bodyScratch=reinterpret_cast<i32>(list);return bodyScratch;
}
#include "projectile_collision_runtime_test.h"
int main() {
    p1.mHealth=p2.mHealth=100;gCoOpPlayers.router.Publish(COOP_PLAYER_ONE,&p1);gCoOpPlayers.router.Publish(COOP_PLAYER_TWO,&p2);
    CVector from={0,0,0},to={100,0,0},hit={-1,-1,-1};
    contact[1]=true;hitX[1]=60;
    assert(CoOp_PlayerLineToSphere(&from,&to,&hit,&p1,0,4096)==reinterpret_cast<i32>(&p2) && hit.vx==60);
    assert(queries[0]==1 && queries[1]==1 && gCoOpProjectileHits[1]==1);
    contact[0]=true;hitX[0]=20;
    assert(CoOp_PlayerLineToSphere(&from,&to,&hit,&p1,0,4096)==reinterpret_cast<i32>(&p1) && hit.vx==20);
    assert(bodyScratch==reinterpret_cast<i32>(&p1) && projectionScratch==50 && mathScratch[0]==1 && mathScratch[255]==1);
    hitX[1]=10;assert(CoOp_PlayerLineToSphere(&from,&to,&hit,&p2,0,4096)==reinterpret_cast<i32>(&p2) && hit.vx==10);
    p2.mHealth=0;assert(CoOp_PlayerLineToSphere(&from,&to,&hit,&p1,0,4096)==reinterpret_cast<i32>(&p1));
    p2.mHealth=100;hidden[1]=true;assert(CoOp_PlayerLineToSphere(&from,&to,&hit,&p1,0,4096)==reinterpret_cast<i32>(&p1));hidden[1]=false;
    assert(CoOp_PlayerLineToSphere(&from,&to,&hit,&p1,&p2,4096)==reinterpret_cast<i32>(&p1));
    contact[0]=contact[1]=false;hit.vx=777;
    assert(!CoOp_PlayerLineToSphere(&from,&to,&hit,&p1,0,4096) && hit.vx==777);
    CBody prop;assert(CoOp_PlayerLineToSphere(&from,&to,&hit,&prop,0,4096)==123 && otherCalls==1);
    gCoOpSession.enabled=false;contact[1]=true;
    unsigned n=queries[1];assert(!CoOp_PlayerLineToSphere(&from,&to,&hit,&p1,0,4096) && queries[1]==n);
    gCoOpSession.enabled=true;gCoOpPlayers.router.Withdraw(COOP_PLAYER_TWO);
    assert(!CoOp_PlayerLineToSphere(&from,&to,&hit,&p1,0,4096) && queries[1]==n);
    puts("PASS native projectile hit query: both players, nearest one-hit result/point, dead/hidden/ignored, miss, other lists, singleplayer");return 0;
}
