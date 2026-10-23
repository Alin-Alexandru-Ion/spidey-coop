// Co-op / player: Construct, update, carry, and recover each independent retail CPlayer.
#pragma once
#ifndef COOP_ENTRY_STATE_H
#define COOP_ENTRY_STATE_H
#include "spidey.h"
#include "camera.h"

// A freshly constructed CPlayer owns its model, pose buffers, controller,
// inventory, effect list and synthesized-command allocations. Transfer only
// native motion/presentation data at a shared event's control handoff.
static void CoOp_CopyEntryAnimation(CPlayer* to,const CPlayer* from) {
    if(to->mAnim!=from->mAnim) {
        reinterpret_cast<void (FASTCALL*)(CPlayer*,void*,i32,i32,i32)>(0x004C9130)(
            to,0,from->mAnim,from->mFrame,from->mTargetFrame);
    }
    to->mFrame=from->mFrame;to->mFrameFrac=from->mFrameFrac;
    to->mAnimMode=from->mAnimMode;to->mAnimDir=from->mAnimDir;
    to->mAnimFinished=from->mAnimFinished;to->mTargetFrame=from->mTargetFrame;
    to->mAnimSpeed=from->mAnimSpeed;
    // Rebuild using this costume's own animation resource/pose buffers.
    to->mDecompressedAnim=0xffff;to->mDecompressedFrame=-1;
}
// Update shadow entry on the selected CPlayer's independent lifecycle.
static void CoOp_ShadowEntry(CPlayer* to,const CPlayer* from) {
    to->mPos=from->mPos;to->mAngles=from->mAngles;
    to->mTransform=from->mTransform;to->mVel=from->mVel;
    to->mAcc=from->mAcc;CoOp_CopyEntryAnimation(to,from);
}
// Transfer authored movement and animation to the joining player.
static bool CoOp_AdoptEntryMotion(CPlayer* to,const CPlayer* from,const CVector& position) {
    if(!to || !from || to==from || from->field_1AC)return false;
    // An enemy restraint belongs to its victim, not to the shared entry or
    // cinematic. Copying it gives the partner a hold with no grabber/release.
    if(from->mHealth<=0 || (from->field_E1C & 0x30000000))return false;
    u8* target=reinterpret_cast<u8*>(to);
    const u8* source=reinterpret_cast<const u8*>(from);
    typedef void* (__cdecl *AllocFn)(u32);
    typedef void (__cdecl *FreeFn)(void*);
    void* path=0;void* swinger=0;
    const void* sourcePath=*reinterpret_cast<void* const*>(source+0xC90);
    const u8* sourceSwing=*reinterpret_cast<u8* const*>(source+0xE64);
    // Retail look-around motion allocates 24 VECTORs (004C3728); never share
    // that array because ExitLookaroundMode/DeleteStuff frees it.
    if(sourcePath) {
        path=reinterpret_cast<AllocFn>(0x00455390)(0x180);
        if(!path)return false;
        memcpy(path,sourcePath,0x180);
    }
    if(sourceSwing) {
        swinger=reinterpret_cast<AllocFn>(0x00455390)(0x190);
        if(!swinger) {
            if(path)reinterpret_cast<FreeFn>(0x00458210)(path);
            return false;
        }
        memset(swinger,0,0x190);
        // The same CSwinger constructor used at 004B3D7A creates a private
        // web and geometry. Its numeric pendulum state follows below; +17C
        // is the new owned web pointer and must stay untouched.
        reinterpret_cast<void* (FASTCALL*)(void*,void*,const void*,i32,const void*,const void*)>(0x004F6F00)(
            swinger,0,source+0xD64,*reinterpret_cast<const i32*>(source+0xD7C),
            source+0xD80,source+0xDA0);
        memcpy(static_cast<u8*>(swinger)+0xF4,sourceSwing+0xF4,0x88);
        memcpy(static_cast<u8*>(swinger)+0x180,sourceSwing+0x180,0x10);
    }
    void* oldPath=*reinterpret_cast<void**>(target+0xC90);
    void* oldSwing=*reinterpret_cast<void**>(target+0xE64);
    if(oldPath)reinterpret_cast<FreeFn>(0x00458210)(oldPath);
    if(oldSwing) {
        typedef void* (FASTCALL *DeleteFn)(void*,void*,u32);
        reinterpret_cast<DeleteFn>((*reinterpret_cast<u32**>(oldSwing))[0])(oldSwing,0,1);
    }
    // Retail layout audited against CPlayer/CSuper definitions and the native
    // first update (004B1999), surface collision (00466CE0) and motion AI.
    // Collision packets contain borrowed world item/face pointers. Both bodies
    // are in the same bank; no model/effect/script/target ownership is copied.
    const unsigned spans[][2]={
        {0x48,0x34}, {0x88,0x26},             // linear/angular physics, surface normal
        {0x500,8}, {0x53C,0x11}, {0x553,1}, // input history, camera initialization, first-floor snap
        {0x89C,0x28}, {0x8E8,8},            // surface basis and wall/ceiling/look flags
        {0xAC8,0x1F}, {0xB08,1}, {0xB0C,0x148}, // contact vectors and two SLineInfo packets
        {0xC6C,0x24}, {0xC94,0x128},        // heading, look/zip/swing numerical parameters; excludes C90
        {0xDC0,0xC}, {0xDF0,0x1C},         // anchor and torso/angular movement; excludes target DCC
        {0xE18,0xC}, {0xE2D,0x13},         // ignore-input timer, native state/counter and analogue heading
        {0xE80,0x14}                        // jump/fall speed and crawl acceleration
    };
    for(unsigned i=0;i<sizeof(spans)/sizeof(spans[0]);++i)
        memcpy(target+spans[i][0],source+spans[i][0],spans[i][1]);
    *reinterpret_cast<void**>(target+0xC90)=path;
    *reinterpret_cast<void**>(target+0xE64)=swinger;
    to->mCollision=from->mCollision;
    CoOp_ShadowEntry(to,from);to->mPos=position;
    to->field_7C=*reinterpret_cast<i32*>(0x006B4CA8);
    return true;
}
// Transfer authored camera mode and pose to the joining player.
static void CoOp_AdoptEntryCamera(CCamera* to,const CCamera* from) {
    if(!to || !from)return;
    // Native per-view tuning/interpolation and orientation are values. Keep
    // tripod +FC, target +13C, fade lists and all ownership on this camera.
    const unsigned spans[][2]={{0x104,0x38},{0x144,0x3C},{0x1A8,0x38},
        {0x1E4,0x58},{0x264,0xC},{0x280,0x24}};
    for(unsigned i=0;i<sizeof(spans)/sizeof(spans[0]);++i)
        memcpy(reinterpret_cast<u8*>(to)+spans[i][0],
            reinterpret_cast<const u8*>(from)+spans[i][0],spans[i][1]);
    reinterpret_cast<void (FASTCALL*)(CCamera*,void*)>(0x00416AA0)(to,0);
}
#endif
