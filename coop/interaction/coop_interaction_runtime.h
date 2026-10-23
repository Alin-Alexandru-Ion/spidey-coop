// Co-op / interaction: Adapt native enemy, projectile, boss, and object interactions.
#pragma once
#ifndef COOP_INTERACTION_RUNTIME_H
#define COOP_INTERACTION_RUNTIME_H
static bool gCoOpInteractionsInstalled;
static bool gCoOpCameraUpdateInstalled;
static unsigned gCoOpPickupCount[2],gCoOpEnemyCount[2];
static unsigned gCoOpChopperTargets[2];
static unsigned gCoOpChaseTargets[2];
static unsigned gCoOpBulletTargets[2];
static unsigned gCoOpRhinoTargets[2],gCoOpVenomTargets[2],gCoOpLizManTargets[2];
static CoOpPlayerHandle gCoOpVenomDisplayOwner;
static CoOpPlayerHandle gCoOpVenomGrabOwner;
static CVenom* gCoOpVenomGrabBoss;
struct CoOpGrabLease {
    CoOpPlayerHandle victim;SHandle enemy;
    u32 state,phase,callback,enemyHoldFlags,holdCallback,enemyRenderFlags;
    void* wrap;
    // Initialize co-op grab lease with no active retail ownership.
    CoOpGrabLease():state(0),phase(0),callback(0),enemyHoldFlags(0),holdCallback(0),enemyRenderFlags(0),wrap(0){memset(&enemy,0,sizeof(enemy));}
};
static CoOpGrabLease gCoOpGrab[2];
// Route grab callback through the interacting player's native context.
static bool CoOp_GrabCallback(u32 callback) {
    // Native Venom wrap, hand grab and tug hold all retain the victim.
    // The wrap callback can pin a player whose state is already airborne.
    return callback==0x004F0A70 || callback==0x004F0CA0 || callback==0x004F0D70;
}
// Resolve a captured enemy handle before reading its current grab state.
static CBaddy* CoOp_Grabber(const CoOpGrabLease& grab) {
    return reinterpret_cast<CBaddy* (__cdecl*)(const SHandle*)>(0x004583F0)(&grab.enemy);
}
// Restore the victim's native damage and animation state after a web interruption.
static void CoOp_ReleaseInterruptedGrab(CPlayer* player,CoOpGrabLease& grab) {
    u8* bytes=reinterpret_cast<u8*>(player);
    CBaddy* enemy=CoOp_Grabber(grab);
    bool held=player->field_E1C==grab.state;
    u32& callback=*reinterpret_cast<u32*>(bytes+0x554);
    bool pinned=grab.state==0x20000000 && CoOp_GrabCallback(callback);
    if((held || pinned) && player->mHealth>0 && enemy && enemy->mHealth>0 &&
       enemy->field_31C.bothFlags==grab.phase)return;
    // A restraint owns state on BOTH participants. Venom sets 218:4000 when
    // installing a hold callback; Hit rejects all damage while that bit is set.
    // Web interruption switches phase without visiting the native grab exit
    // that clears it. End only the flags retained by this verified lease.
    if(enemy && grab.enemyHoldFlags) {
        bool transferred=false;
        for(unsigned i=0;i<2;++i) {
            CoOpGrabLease& other=gCoOpGrab[i];
            const CoOpPlayerRouter::Participant* victim=gCoOpPlayers.Router().Resolve(other.victim);
            if(&other==&grab || !other.state || CoOp_Grabber(other)!=enemy || !victim ||
               victim->player->mHealth<=0 || other.phase!=enemy->field_31C.bothFlags)continue;
            const u32 pin=*reinterpret_cast<const u32*>(reinterpret_cast<const u8*>(victim->player)+0x554);
            if(victim->player->field_E1C==other.state ||
               (other.state==0x20000000 && CoOp_GrabCallback(pin))) {
                other.enemyHoldFlags|=grab.enemyHoldFlags;
                other.enemyRenderFlags|=grab.enemyRenderFlags;
                if(!other.wrap)other.wrap=grab.wrap;
                transferred=true;break;
            }
        }
        if(!transferred) {
            enemy->field_218&=~grab.enemyHoldFlags;
            if(grab.holdCallback==0x004F0A70 || grab.wrap) {
                u8* boss=reinterpret_cast<u8*>(enemy);
                void*& wrap=*reinterpret_cast<void**>(boss+0x3E4);
                // Mirror the native wrap exit (004EF182 / 004EF311). Die
                // queues the effect for removal; it must not be freed twice.
                if(grab.wrap && wrap==grab.wrap) {
                    reinterpret_cast<void (__fastcall*)(void*,void*)>(0x00408930)(wrap,0);
                    wrap=0;
                }
                enemy->field_194&=~grab.enemyRenderFlags;
            }
        }
    }
    // Venom installs a callback that pins the victim to his animated hand.
    // Restore it even if damage/death has already changed the player state.
    if(pinned)callback=grab.callback;
    if((held || pinned) && player->mHealth>0 && !player->field_1AC) {
        bytes[0x564]=0;player->mFlags&=~0x800;
        player->mVel.vx=player->mVel.vy=player->mVel.vz=0;
        player->mAcc.vx=player->mAcc.vy=player->mAcc.vz=0;
        // Native living Venom release enters this airborne animation/state.
        // Let retail physics find the floor rather than forcing a ground pose.
        player->field_E1C=0x800000;
        int animation=grab.holdCallback==0x004F0A70?176:172;
        reinterpret_cast<void (__fastcall*)(CPlayer*,void*,int,int,int)>(0x004C9130)(player,0,animation,0,-1);
    }
    grab=CoOpGrabLease();
}
// Route clean grab through the interacting player's native context.
static void CoOp_CleanGrab(void* argument) {
    unsigned slot=*static_cast<unsigned*>(argument);
    CPlayer* player=gCoOpPlayers.Get((CoOpPlayerSlot)slot);
    const CoOpPlayerRouter::Participant* victim=gCoOpPlayers.Router().Resolve(gCoOpGrab[slot].victim);
    if(player && victim && victim->player==player)
        CoOp_ReleaseInterruptedGrab(player,gCoOpGrab[slot]);
    else gCoOpGrab[slot]=CoOpGrabLease();
}
// Inspect active grabs and release victims after an interrupted attacker.
static void CoOp_GrabFrame() {
    // Cleanup cannot depend on a webbed/deactivated/deleted grabber being
    // selected or receiving another AI tick. Each lease owns one generation
    // of one player, even when both players happen to have the same state.
    if(!gCoOpPlayers.Get(COOP_PLAYER_TWO) || gCoOpPlayers.InSecondaryContext())return;
    for(unsigned slot=0;slot<2;++slot)if(gCoOpGrab[slot].state) {
        if(slot==COOP_PLAYER_TWO)gCoOpPlayers.WithSecondaryContext(CoOp_CleanGrab,&slot);
        else CoOp_CleanGrab(&slot);
    }
}
// Route grab target through the interacting player's native context.
static CoOpPlayerHandle CoOp_GrabTarget(CBaddy* enemy,CoOpPlayerHandle fallback) {
    for(unsigned i=0;i<2;++i)if(gCoOpGrab[i].state && CoOp_Grabber(gCoOpGrab[i])==enemy) {
        if(i==COOP_PLAYER_TWO)gCoOpPlayers.WithSecondaryContext(CoOp_CleanGrab,&i);
        else CoOp_CleanGrab(&i);
        if(!gCoOpGrab[i].state)continue;
        const CoOpPlayerRouter::Participant* victim=gCoOpPlayers.Router().Resolve(gCoOpGrab[i].victim);
        if(victim && victim->player->mHealth>0)return gCoOpGrab[i].victim;
        if(i==COOP_PLAYER_TWO)gCoOpPlayers.WithSecondaryContext(CoOp_CleanGrab,&i);
        else CoOp_CleanGrab(&i);
    }
    return fallback;
}
// Route enemy alive through the interacting player's native context.
static bool CoOp_EnemyAlive(CoOpPlayerHandle owner,const CPlayer* player,void*) {
    if(gCoOpScriptMotion.Partner(owner.slot))return false;
    typedef i32 (__fastcall *Fn)(const CPlayer*,void*);
    return !reinterpret_cast<Fn>(0x00460700)(player,0);
}
// Route enemy visible through the interacting player's native context.
static bool CoOp_EnemyVisible(CoOpPlayerHandle h,const CPlayer* player,void* context) {
    if(!CoOp_EnemyAlive(h,player,context)) return false;
    CBaddy* enemy=static_cast<CBaddy*>(context);
    typedef i32 (__cdecl *Fn)(const CVector*,const CVector*,i32,i32);
    return reinterpret_cast<Fn>(0x004E67A0)(&enemy->mPos,&player->mPos,0,0)!=0;
}
struct CoOpEnemyCall {CBaddy* enemy;u32 original;};
// Route run enemy through the interacting player's native context.
static void CoOp_RunEnemy(void* argument) {
    CoOpEnemyCall* call=static_cast<CoOpEnemyCall*>(argument);
    // DistanceToPlayer caches a value without recording which player it measured.
    // Invalidate it before the original AI uses the newly selected target.
    call->enemy->field_208=0;
    CPlayer* player=*reinterpret_cast<CPlayer**>(0x006A9038);
    unsigned slot=player==gCoOpPlayers.Get(COOP_PLAYER_ONE)?0:1;
    CoOpGrabLease& grab=gCoOpGrab[slot];
    if(grab.state && gCoOpPlayers.Router().Resolve(grab.victim))CoOp_ReleaseInterruptedGrab(player,grab);
    else grab=CoOpGrabLease();
    u32 before=player->field_E1C;
    u32 callback=*reinterpret_cast<u32*>(reinterpret_cast<u8*>(player)+0x554);
    u32 renderFlags=call->enemy->mType==313?call->enemy->field_194:0;
    typedef void (__fastcall *Fn)(CBaddy*,void*);
    reinterpret_cast<Fn>(call->original)(call->enemy,0);
    u32 expected=call->enemy->mType==317?0x10000000:call->enemy->mType==313?0x20000000:0;
    u32 afterCallback=*reinterpret_cast<u32*>(reinterpret_cast<u8*>(player)+0x554);
    // RunAnim installs a command table; its held-state instruction may run
    // on the NEXT player update. Capture the callback installation now,
    // rather than waiting for a state transition inside the enemy AI call.
    bool installsHold=expected==0x20000000 && CoOp_GrabCallback(afterCallback) &&
        !CoOp_GrabCallback(callback);
    if(!grab.state && expected && ((player->field_E1C==expected && before!=expected) || installsHold)) {
        grab.victim=gCoOpPlayers.Router().Handle((CoOpPlayerSlot)slot);
        grab.enemy=reinterpret_cast<SHandle (__cdecl*)(void*)>(0x00458360)(call->enemy);
        grab.state=expected;grab.phase=call->enemy->field_31C.bothFlags;grab.callback=callback;
        grab.holdCallback=afterCallback;
        grab.enemyHoldFlags=expected==0x20000000 ? call->enemy->field_218 & 0x4000:0;
        if(afterCallback==0x004F0A70 && expected==0x20000000) {
            u8* boss=reinterpret_cast<u8*>(call->enemy);
            grab.wrap=*reinterpret_cast<void**>(boss+0x3E4);
            grab.enemyRenderFlags=call->enemy->field_194&~renderFlags&0x4800;
        }
    }
    if(grab.state)CoOp_ReleaseInterruptedGrab(player,grab);
}
// Select the nearest eligible player before one native enemy AI tick.
static void CoOp_EnemyAI(CBaddy* enemy,u32 original) {
    typedef void (__fastcall *Fn)(CBaddy*,void*);
    if(!gCoOpPlayers.Get(COOP_PLAYER_TWO) || gCoOpPlayers.InSecondaryContext()) {
        reinterpret_cast<Fn>(original)(enemy,0);return;
    }
    CoOpPlayerHandle target=CoOp_SelectEnemy(gCoOpPlayers.Router(),enemy->mPos,
        CoOp_EnemyVisible,CoOp_EnemyAlive,enemy);
    if(!gCoOpPlayers.Router().Resolve(target)) {
        reinterpret_cast<Fn>(original)(enemy,0);return;
    }
    target=CoOp_GrabTarget(enemy,target);
    CoOpEnemyCall call={enemy,original};++gCoOpEnemyCount[target.slot];
    if(enemy->mType==307)++gCoOpRhinoTargets[target.slot];
    if(enemy->mType==317)++gCoOpLizManTargets[target.slot];
    if(target.slot==COOP_PLAYER_TWO) gCoOpPlayers.WithSecondaryContext(CoOp_RunEnemy,&call);
    else CoOp_RunEnemy(&call);
}
static void __fastcall CoOp_ThugAI(CThug* enemy,void*) {CoOp_EnemyAI(enemy,0x004DB280);}
static void __fastcall CoOp_CopAI(CCop* enemy,void*) {CoOp_EnemyAI(enemy,0x0042FA10);}
static void __fastcall CoOp_LizManAI(CLizMan* enemy,void*) {CoOp_EnemyAI(enemy,0x00451350);}
static void __fastcall CoOp_RhinoAI(CRhino* boss,void*) {CoOp_EnemyAI(boss,0x004819C0);}
static void __fastcall CoOp_VenomAI(CVenom* boss,void*) {
    typedef void (__fastcall *Fn)(CVenom*,void*);
    Fn original=reinterpret_cast<Fn>(0x004EC040);
    if(!gCoOpPlayers.Get(COOP_PLAYER_TWO) || gCoOpPlayers.InSecondaryContext()) {
        gCoOpVenomGrabOwner=CoOpPlayerHandle();gCoOpVenomGrabBoss=0;
        original(boss,0);return;
    }
    // Venom's chase logic reads MechList during the single native AI tick.
    // Follow the living player nearest Venom, including one behind scenery;
    // line of sight is irrelevant to progress in this authored race.
    CoOpPlayerRouter::Query query;query.origin=boss->mPos;
    query.condition=CoOp_EnemyAlive;
    CoOpPlayerHandle target=gCoOpPlayers.Router().Select(query);
    // Native grab phase 0x100 drives the victim through MechList every tick.
    // Retargeting midway leaves the former victim suspended in grab animation.
    const CoOpPlayerRouter::Participant* grabVictim=
        gCoOpPlayers.Router().Resolve(gCoOpVenomGrabOwner);
    if(boss->field_31C.bothFlags==0x100 && boss==gCoOpVenomGrabBoss &&
        grabVictim && CoOp_EnemyAlive(gCoOpVenomGrabOwner,grabVictim->player,0))
        target=gCoOpVenomGrabOwner;
    if(!gCoOpPlayers.Router().Resolve(target)) {original(boss,0);return;}
    target=CoOp_GrabTarget(boss,target);
    gCoOpVenomDisplayOwner=target;
    CoOpEnemyCall call={boss,0x004EC040};++gCoOpEnemyCount[target.slot];
    ++gCoOpVenomTargets[target.slot];
    if(target.slot==COOP_PLAYER_TWO) {
        if(!gCoOpPlayers.WithSecondaryContext(CoOp_RunEnemy,&call)) {
            original(boss,0);target=gCoOpPlayers.Router().Handle(COOP_PLAYER_ONE);
        }
    } else CoOp_RunEnemy(&call);
    if(boss->field_31C.bothFlags==0x100) {
        gCoOpVenomGrabBoss=boss;gCoOpVenomGrabOwner=target;
    } else {
        gCoOpVenomGrabBoss=0;gCoOpVenomGrabOwner=CoOpPlayerHandle();
    }
}
struct CoOpVenomProgressCall {const u32* input;u32* output;};
// Route run venom progress through the interacting player's native context.
static void CoOp_RunVenomProgress(void* argument) {
    CoOpVenomProgressCall* call=static_cast<CoOpVenomProgressCall*>(argument);
    CoOp_HudRaceProgress(0x004E7E10,call->input,call->output);
}
// Route venom progress through the interacting player's native context.
static void CoOp_VenomProgress(const u32* input,u32* output) {
    CoOpVenomProgressCall call={input,output};
    if(gCoOpSession.enabled && gCoOpVenomDisplayOwner.slot==COOP_PLAYER_TWO &&
        gCoOpPlayers.Router().Resolve(gCoOpVenomDisplayOwner) &&
        gCoOpPlayers.WithSecondaryContext(CoOp_RunVenomProgress,&call))return;
    CoOp_RunVenomProgress(&call);
}
// Route venom reloc init through the interacting player's native context.
static void __cdecl CoOp_VenomRelocInit(reloc_mod* module) {
    reinterpret_cast<void (__cdecl*)(reloc_mod*)>(0x004E7D30)(module);
    if(module && module->field_C[1]==reinterpret_cast<UserFunction>(0x004E7E10))
        module->field_C[1]=CoOp_VenomProgress;
}
// Route l2 race progress through the interacting player's native context.
static void CoOp_L2RaceProgress(const u32* input,u32* output) {
    CoOp_HudRaceProgress(0x004477F0,input,output);
}
// Route l2 race reloc init through the interacting player's native context.
static void __cdecl CoOp_L2RaceRelocInit(reloc_mod* module) {
    reinterpret_cast<void (__cdecl*)(reloc_mod*)>(0x00447640)(module);
    if(module && module->field_C[1]==reinterpret_cast<UserFunction>(0x004477F0))
        module->field_C[1]=CoOp_L2RaceProgress;
}
// Route l5 race progress through the interacting player's native context.
static void CoOp_L5RaceProgress(const u32* input,u32* output) {
    CoOp_HudRaceProgress(0x004489B0,input,output);
}
// Route l5 race reloc init through the interacting player's native context.
static void __cdecl CoOp_L5RaceRelocInit(reloc_mod* module) {
    reinterpret_cast<void (__cdecl*)(reloc_mod*)>(0x00448730)(module);
    if(module && module->field_C[1]==reinterpret_cast<UserFunction>(0x004489B0))
        module->field_C[1]=CoOp_L5RaceProgress;
}
// Route super ock race progress through the interacting player's native context.
static void CoOp_SuperOckRaceProgress(const u32* input,u32* output) {
    CoOp_HudRaceProgress(0x004D0E70,input,output);
}
// Route super ock race reloc init through the interacting player's native context.
static void __cdecl CoOp_SuperOckRaceRelocInit(reloc_mod* module) {
    reinterpret_cast<void (__cdecl*)(reloc_mod*)>(0x004CBD70)(module);
    if(module && module->field_C[1]==reinterpret_cast<UserFunction>(0x004D0E70))
        module->field_C[1]=CoOp_SuperOckRaceProgress;
}
// Install install race progress hooks at the supported retail call site, preserving its fallback.
static bool CoOp_InstallRaceProgressHooks() {
    u32* sites[4]={reinterpret_cast<u32*>(0x00551BA4),reinterpret_cast<u32*>(0x00551C44),
        reinterpret_cast<u32*>(0x00551C80),reinterpret_cast<u32*>(0x00551D98)};
    const u32 expected[4]={0x00447640,0x004E7D30,0x00448730,0x004CBD70};
    const u32 hooks[4]={reinterpret_cast<u32>(&CoOp_L2RaceRelocInit),
        reinterpret_cast<u32>(&CoOp_VenomRelocInit),reinterpret_cast<u32>(&CoOp_L5RaceRelocInit),
        reinterpret_cast<u32>(&CoOp_SuperOckRaceRelocInit)};
    unsigned i;
    for(i=0;i<4;++i)if(*sites[i]!=expected[i])return false;
    DWORD old,unused;
    if(!VirtualProtect(sites[0],0x1F8,PAGE_READWRITE,&old))return false;
    for(i=0;i<4;++i)*sites[i]=hooks[i];
    VirtualProtect(sites[0],0x1F8,old,&unused);
    return true;
}
static unsigned gCoOpChaseActorPulses[2];
// Route route chase actor pulses through the interacting player's native context.
static void CoOp_RouteChaseActorPulses(unsigned before,CoOpPlayerHandle target) {
    if(!gCoOpPlayers.Router().Resolve(target))return;
    PendingListEntry* queue=reinterpret_cast<PendingListEntry*>(0x006B4688);
    unsigned after=CoOp_PendingCount();
    for(unsigned i=before;i<after;++i) {
        gCoOpDeferredOwners.Record(queue[i].pCommands,queue[i].NodeIndex,target);
        gCoOpNodeOwners.Record(queue[i].NodeIndex,1u<<target.slot);
        ++gCoOpChaseActorPulses[target.slot];
    }
}
static void __fastcall CoOp_ChopperAI(CChopper* chopper,void*) {
    typedef void (__fastcall *Fn)(CChopper*,void*);
    Fn original=reinterpret_cast<Fn>(0x004262A0);
    if(!gCoOpPlayers.Get(COOP_PLAYER_TWO) || gCoOpPlayers.InSecondaryContext()) {
        original(chopper,0);return;
    }
    // Its waypoint and firing code consults MechList. Keep one shared AI tick
    // while choosing a living participant; a dead P1 must not remain the aim.
    CoOpPlayerRouter::Query query;query.origin=chopper->mPos;
    query.condition=CoOp_EnemyAlive;
    CoOpPlayerHandle target=gCoOpPlayers.Router().Select(query);
    unsigned before=CoOp_PendingCount();
    if(target.slot==COOP_PLAYER_TWO) {
        CoOpEnemyCall call={chopper,0x004262A0};
        ++gCoOpChopperTargets[1];
        if(!gCoOpPlayers.WithSecondaryContext(CoOp_RunEnemy,&call)) {original(chopper,0);target=gCoOpPlayers.Router().Handle(COOP_PLAYER_ONE);}
    } else {
        if(target.slot==COOP_PLAYER_ONE)++gCoOpChopperTargets[0];
        original(chopper,0);
    }
    CoOp_RouteChaseActorPulses(before,target);
}
struct CoOpChaseBodyCall {CBody* body;u32 original;};
// Route run chase body through the interacting player's native context.
static void CoOp_RunChaseBody(void* argument) {
    CoOpChaseBodyCall* call=static_cast<CoOpChaseBodyCall*>(argument);
    reinterpret_cast<void (__fastcall*)(CBody*,void*)>(call->original)(call->body,0);
}
// Route chase body ai through the interacting player's native context.
static void CoOp_ChaseBodyAI(CBody* body,u32 original) {
    typedef void (__fastcall *Fn)(CBody*,void*);
    if(!gCoOpPlayers.Get(COOP_PLAYER_TWO) || gCoOpPlayers.InSecondaryContext()) {
        reinterpret_cast<Fn>(original)(body,0);return;
    }
    CoOpPlayerRouter::Query query;query.origin=body->mPos;
    query.condition=CoOp_EnemyAlive;
    CoOpPlayerHandle target=gCoOpPlayers.Router().Select(query);
    unsigned before=CoOp_PendingCount();
    if(target.slot==COOP_PLAYER_TWO) {
        CoOpChaseBodyCall call={body,original};++gCoOpChaseTargets[1];
        if(!gCoOpPlayers.WithSecondaryContext(CoOp_RunChaseBody,&call)) {
            reinterpret_cast<Fn>(original)(body,0);target=gCoOpPlayers.Router().Handle(COOP_PLAYER_ONE);
        }
    } else {
        if(target.slot==COOP_PLAYER_ONE)++gCoOpChaseTargets[0];
        reinterpret_cast<Fn>(original)(body,0);
    }
    CoOp_RouteChaseActorPulses(before,target);
}
static void __fastcall CoOp_SniperAI(CSniperTarget* actor,void*) {CoOp_ChaseBodyAI(actor,0x00421900);}
static void __fastcall CoOp_SearchlightAI(CSearchlight* actor,void*) {CoOp_ChaseBodyAI(actor,0x00422EF0);}
static void __fastcall CoOp_MissileAI(CChopperMissile* actor,void*) {CoOp_ChaseBodyAI(actor,0x00425400);}
struct CoOpBulletCall {CMachineGunBullet* bullet;};
// Route run bullet through the interacting player's native context.
static void CoOp_RunBullet(void* argument) {
    CoOpBulletCall* call=static_cast<CoOpBulletCall*>(argument);
    reinterpret_cast<void (__fastcall*)(CMachineGunBullet*,void*)>(0x00421100)(call->bullet,0);
}
static void __fastcall CoOp_BulletMove(CMachineGunBullet* bullet,void*) {
    // Chopper and sniper AI only aim the projectile. Its later Move tick reads
    // MechList again for collision, so route that tick to the living body at
    // the projectile's current end point without simulating it twice.
    if(!gCoOpPlayers.Get(COOP_PLAYER_TWO) || gCoOpPlayers.InSecondaryContext()) {
        CoOpBulletCall call={bullet};CoOp_RunBullet(&call);return;
    }
    CoOpPlayerRouter::Query query;query.origin=bullet->mEnd;
    query.condition=CoOp_EnemyAlive;
    CoOpPlayerHandle target=gCoOpPlayers.Router().Select(query);
    CoOpBulletCall call={bullet};
    if(target.slot==COOP_PLAYER_TWO) {
        ++gCoOpBulletTargets[1];
        if(!gCoOpPlayers.WithSecondaryContext(CoOp_RunBullet,&call))CoOp_RunBullet(&call);
    } else {
        if(target.slot==COOP_PLAYER_ONE)++gCoOpBulletTargets[0];
        CoOp_RunBullet(&call);
    }
}
static void __fastcall CoOp_ScorpionAI(CScorpion* boss,void*) {
    typedef void (__fastcall *Fn)(CScorpion*,void*);
    Fn original=reinterpret_cast<Fn>(0x00488590);
    // Scorpion also has an authored Jonah target. Only retarget his explicit
    // player phase; intro/hostage scripts keep their native target and timing.
    if(!gCoOpPlayers.Get(COOP_PLAYER_TWO) || gCoOpPlayers.InSecondaryContext() ||
        boss->field_BF8!=2) {original(boss,0);return;}
    CoOpPlayerHandle target=CoOp_SelectEnemy(gCoOpPlayers.Router(),boss->mPos,
        CoOp_EnemyVisible,CoOp_EnemyAlive,boss);
    const CoOpPlayerRouter::Participant* participant=gCoOpPlayers.Router().Resolve(target);
    if(!participant) {original(boss,0);return;}
    // Both the retained handle and legacy MechList users in tail/hit code must
    // agree on the same recipient for this single native AI tick.
    typedef SHandle (__cdecl *HandleFn)(void*);
    boss->hCurrentTarget=reinterpret_cast<HandleFn>(0x00458360)(participant->player);
    CoOpEnemyCall call={boss,0x00488590};++gCoOpEnemyCount[target.slot];
    if(target.slot==COOP_PLAYER_TWO) gCoOpPlayers.WithSecondaryContext(CoOp_RunEnemy,&call);
    else CoOp_RunEnemy(&call);
}
static bool __fastcall CoOp_Pickup(CPowerUp* pickup,void*,CPlayer* collector) {
    typedef bool (__fastcall *Fn)(CPowerUp*,void*,CPlayer*);
    if(!gCoOpPlayers.Get(COOP_PLAYER_TWO))
        return reinterpret_cast<Fn>(0x0046B860)(pickup,0,collector);
    CoOpPlayerHandle owner=CoOp_Collector(gCoOpPlayers.Router(),collector);
    if(!gCoOpPlayers.Router().Resolve(owner)) return false;
    unsigned previous=gCoOpTriggerOwner;gCoOpTriggerOwner=1u<<owner.slot;
    unsigned before=CoOp_PendingCount();
    bool question=pickup->mType==11;
    bool result=reinterpret_cast<Fn>(0x0046B860)(pickup,0,collector);
    // The pickup may have entered its death state. Do not read it after the call.
    if(question) {
        PendingListEntry* queue=reinterpret_cast<PendingListEntry*>(0x006B4688);
        unsigned after=CoOp_PendingCount();
        for(unsigned i=before;i<after;++i) {
            gCoOpDeferredOwners.Record(queue[i].pCommands,queue[i].NodeIndex,owner);
            gCoOpNodeOwners.Record(queue[i].NodeIndex,1u<<owner.slot);
            ++gCoOpPickupCount[owner.slot];
        }
    }
    gCoOpTriggerOwner=previous;return result;
}
// Install install camera update hook at the supported retail call site, preserving its fallback.
static bool CoOp_InstallCameraUpdateHook() {
    const u8* cameraCall=reinterpret_cast<const u8*>(0x004555FC);
    if(cameraCall[0]!=0xE8 || 0x00455601+*reinterpret_cast<const i32*>(cameraCall+1)!=0x00460FC0) return false;
    DWORD protect,ignored;
    if(!VirtualProtect(reinterpret_cast<void*>(0x004555FC),5,PAGE_EXECUTE_READWRITE,&protect)) return false;
    PATCH_CALL(0x004555FC,CoOp_UpdateCameras);
    VirtualProtect(reinterpret_cast<void*>(0x004555FC),5,protect,&ignored);
    FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(0x004555FC),5);
    gCoOpCameraUpdateInstalled=true;return true;
}
// Install install interaction hooks at the supported retail call site, preserving its fallback.
static bool CoOp_InstallInteractionHooks() {
    const u32 sites[]={0x004B7AC3,0x004B7ACD};
    for(unsigned i=0;i<2;++i) {
        const u8* code=reinterpret_cast<const u8*>(sites[i]);
        if(code[0]!=0xE8 || sites[i]+5+*reinterpret_cast<const i32*>(code+1)!=0x0046B860) return false;
    }
    if(*reinterpret_cast<u32*>(0x0053B9B4)!=0x00451350 ||
       *reinterpret_cast<u32*>(0x0053BE34)!=0x00488590 ||
       *reinterpret_cast<u32*>(0x0053BDC4)!=0x004819C0 ||
       *reinterpret_cast<u32*>(0x0053C6C0)!=0x004EC040 ||
       *reinterpret_cast<u32*>(0x0053B678)!=0x0042FA10 ||
       *reinterpret_cast<u32*>(0x0053C558)!=0x004DB280 ||
       *reinterpret_cast<u32*>(0x0053B5F8)!=0x004262A0 ||
       *reinterpret_cast<u32*>(0x0053B5A0)!=0x00421900 ||
       *reinterpret_cast<u32*>(0x0053B5C0)!=0x00422EF0 ||
       *reinterpret_cast<u32*>(0x0053B5E0)!=0x00425400 ||
       *reinterpret_cast<u32*>(0x0053B594)!=0x00421100) return false;
    DWORD textProtect,dataProtect;
    if(!VirtualProtect(reinterpret_cast<void*>(0x004B7000),0x1000,PAGE_EXECUTE_READWRITE,&textProtect)) return false;
    if(!VirtualProtect(reinterpret_cast<void*>(0x0053B000),0x2000,PAGE_READWRITE,&dataProtect)) {
        DWORD ignored;VirtualProtect(reinterpret_cast<void*>(0x004B7000),0x1000,textProtect,&ignored);return false;
    }
    PATCH_CALL(0x004B7AC3,CoOp_Pickup);PATCH_CALL(0x004B7ACD,CoOp_Pickup);
    *reinterpret_cast<u32*>(0x0053B9B4)=reinterpret_cast<u32>(&CoOp_LizManAI);
    *reinterpret_cast<u32*>(0x0053B678)=reinterpret_cast<u32>(&CoOp_CopAI);
    *reinterpret_cast<u32*>(0x0053C558)=reinterpret_cast<u32>(&CoOp_ThugAI);
    *reinterpret_cast<u32*>(0x0053BE34)=reinterpret_cast<u32>(&CoOp_ScorpionAI);
    *reinterpret_cast<u32*>(0x0053BDC4)=reinterpret_cast<u32>(&CoOp_RhinoAI);
    *reinterpret_cast<u32*>(0x0053C6C0)=reinterpret_cast<u32>(&CoOp_VenomAI);
    *reinterpret_cast<u32*>(0x0053B5F8)=reinterpret_cast<u32>(&CoOp_ChopperAI);
    *reinterpret_cast<u32*>(0x0053B5A0)=reinterpret_cast<u32>(&CoOp_SniperAI);
    *reinterpret_cast<u32*>(0x0053B5C0)=reinterpret_cast<u32>(&CoOp_SearchlightAI);
    *reinterpret_cast<u32*>(0x0053B5E0)=reinterpret_cast<u32>(&CoOp_MissileAI);
    *reinterpret_cast<u32*>(0x0053B594)=reinterpret_cast<u32>(&CoOp_BulletMove);
    DWORD ignored;
    VirtualProtect(reinterpret_cast<void*>(0x004B7000),0x1000,textProtect,&ignored);
    VirtualProtect(reinterpret_cast<void*>(0x0053B000),0x2000,dataProtect,&ignored);
    FlushInstructionCache(GetCurrentProcess(),0,0);gCoOpInteractionsInstalled=true;return true;
}
#endif
