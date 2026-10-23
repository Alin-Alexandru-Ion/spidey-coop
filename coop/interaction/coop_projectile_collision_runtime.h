// Co-op / interaction: Adapt native enemy, projectile, boss, and object interactions.

#ifndef COOP_PROJECTILE_COLLISION_RUNTIME_H
#define COOP_PROJECTILE_COLLISION_RUNTIME_H
// A deferred projectile cannot retain a temporary MechList context. Query
// both real participants at impact, then let retail deliver one normal hit.
static bool gCoOpProjectileCollisionInstalled;
static unsigned gCoOpProjectileHits[2];
// Route player line to sphere through the interacting player's native context.
static i32 __cdecl CoOp_PlayerLineToSphere(CVector* from,CVector* to,CVector* hit,
    CBody* list,CBody* ignored,i32 radius) {
    typedef i32 (__cdecl *Fn)(CVector*,CVector*,CVector*,CBody*,CBody*,i32);
    Fn original=reinterpret_cast<Fn>(0x00452C30);
    CPlayer* first=gCoOpPlayers.Get(COOP_PLAYER_ONE);
    CPlayer* second=gCoOpPlayers.Get(COOP_PLAYER_TWO);
    if(!gCoOpSession.enabled || !second || (list!=first && list!=second))
        return original(from,to,hit,list,ignored,radius);
    i32 result=0;double nearest=0;unsigned selected=COOP_PLAYER_COUNT;
    CVector point;u32 projection=0;u8 selectedMath[0x100];
    for(unsigned slot=0;slot<COOP_PLAYER_COUNT;++slot) {
        CPlayer* body=slot?second:first;
        CoOpPlayerHandle owner=gCoOpPlayers.Router().Handle(static_cast<CoOpPlayerSlot>(slot));
        if(!body || body==ignored || !CoOp_EnemyAlive(owner,body,0))continue;
        CVector candidate;candidate.vx=candidate.vy=candidate.vz=0;
        i32 collided=original(from,to,&candidate,body,ignored,radius);
        if(!collided)continue;
        double dx=double(candidate.vx)-from->vx,dy=double(candidate.vy)-from->vy,
            dz=double(candidate.vz)-from->vz;
        double distance=dx*dx+dy*dy+dz*dz;
        if(!result || distance<nearest) {
            result=collided;nearest=distance;point=candidate;selected=slot;
            projection=*reinterpret_cast<u32*>(0x005FBD38);
            memcpy(selectedMath,reinterpret_cast<void*>(0x00610B20),sizeof(selectedMath));
        }
    }
    if(result) {
        *hit=point;++gCoOpProjectileHits[selected];
        // The second query must not leave native hit scratch/GTE describing
        // a different body than the one retail will damage.
        *reinterpret_cast<u32*>(0x005FBD38)=projection;
        *reinterpret_cast<i32*>(0x005FBDE0)=result;
        memcpy(reinterpret_cast<void*>(0x00610B20),selectedMath,sizeof(selectedMath));
    }
    return result;
}
// Install install projectile collision hooks at the supported retail call site, preserving its fallback.
static bool CoOp_InstallProjectileCollisionHooks() {
    // Utils_CheckObjectCollision's player sphere query and LizManSpit's
    // alternate direct sphere query. Native environment fallback stays intact.
    const u32 sites[]={0x004E64A2,0x004521EB};
    for(unsigned i=0;i<2;++i) {
        const u8* code=reinterpret_cast<const u8*>(sites[i]);
        if(code[0]!=0xE8 || sites[i]+5+*reinterpret_cast<const i32*>(code+1)!=0x00452C30)return false;
    }
    DWORD old[2],unused;
    if(!VirtualProtect(reinterpret_cast<void*>(sites[0]),5,PAGE_EXECUTE_READWRITE,&old[0]))return false;
    if(!VirtualProtect(reinterpret_cast<void*>(sites[1]),5,PAGE_EXECUTE_READWRITE,&old[1])) {
        VirtualProtect(reinterpret_cast<void*>(sites[0]),5,old[0],&unused);return false;
    }
    PATCH_CALL(0x004E64A2,CoOp_PlayerLineToSphere);PATCH_CALL(0x004521EB,CoOp_PlayerLineToSphere);
    for(i=0;i<2;++i)VirtualProtect(reinterpret_cast<void*>(sites[i]),5,old[i],&unused);
    FlushInstructionCache(GetCurrentProcess(),0,0);gCoOpProjectileCollisionInstalled=true;return true;
}
#endif
