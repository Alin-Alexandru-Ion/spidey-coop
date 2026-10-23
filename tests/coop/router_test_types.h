#ifndef TEST_SPIDEY_H
#define TEST_SPIDEY_H

#include <windows.h>
typedef unsigned long u32;
typedef long i32;
typedef unsigned char u8; typedef signed char i8; typedef unsigned short u16;
#define FASTCALL __fastcall
struct Vec { i32 vx,vy,vz; };
struct ShortVec {short vx,vy,vz;};
struct TestMatrix {short m[3][3],pad;i32 t[3];};
// Keep the real byte layout: entry/motion/life adapters address native fields.
// A compact lookalike can accidentally alias inventory into physics spans.
struct CSuper {
    void* vtable;
    u16 mFlags,mInquiry;
    Vec mPos;ShortVec mAngles;u16 mModel;
    u8 mDummyFrame,mTintIndex,mDummyAnim,mRegion;
    CSuper* mNextItem;u32 mRGB;ShortVec mScale;u16 scalePad;
    u32 mTRN;CSuper* mPreviousItem;u16 mType,typePad;void* mpLight;
    void* mppOriginalList;u16 mInputFlags,mCBodyFlags;
    Vec field_48,field_54,mVel,mAcc;u32 mFric;
    i32 field_7C,field_80,field_84;
    ShortVec mAngVel,mAngAcc;u32 mAngFric;
    i32 field_98,field_9C,field_A0,field_A4;
    ShortVec field_A8;u8 normalPad[10];
    Vec mShadowPos;ShortVec mShadowNormal;u16 shadowPad;
    void* mpShadow;u16 mShadowScale;short mShadowDist,mShadowThreshold,pushPad;
    i32 mPushVal;u16 mRMinor,mNode,mCollision;short mHealth;
    u16 mPlayerDist,distPad;Vec field_E8;
    i32 field_F4;u16 field_F8,superPad;i32 field_FC,field_100;
    u8 handles[24];void* field_11C;i32 alsoOutlineRelated;
    u8 outlineR,outlineG,outlineB,outlinePad;
    short mFrame;u16 mAnim;u32 mExtraFlags;
    void* mpCalculationOrder;void* mpDecompressedFrame;
    u16 mRoot,mDecompressedAnim;short mDecompressedFrame;
    u8 field_13E,field_13F,mAnimMode; i8 mAnimDir;
    u8 mAnimFinished,field_143;short mTargetFrame,mFrameFrac,mNumFrames,animPad;
    i32 mAnimSpeed;u8 animTail[0x14];TestMatrix mTransform;
    void* mpPoseBuffer;void* mpJoints;void* mpLinks;void* actualcsuperend;
    i32 field_194,field_198;u8 tail[8];
};
typedef Vec CVector;
typedef CSuper CBody;
struct CPlayer: CSuper {u8 playerPad[4];i32 field_1A8;u8 field_1AC;
    u8 routePadding[0x5D4-0x1AD];i32 mWebbing,field_5D8;
    u8 inventoryPad[0x5E9-0x5DC];bool field_5E9;u8 inventoryPadTail[2];i32 field_5EC;
    u8 midPad[0x8EA-0x5F0];u8 field_8EA;
    u8 motionPad[0xE0C-0x8EB];i32 field_E0C;u8 playerTail[0xE1C-0xE10];u32 field_E1C;u8 stateTail[0xEF0-0xE20];i32 mMaxHealth;u8 maxTail[8];};
static void FASTCALL SetArmor(CPlayer* p,int,bool enabled){p->field_5E9=enabled;}
struct CCamera { u32* vtable;u8 bodyPad[0xFC-4];CBody* mTripod;u8 statePadding[0x2A8-0x100];};

#endif
