// Co-op / presentation: Render co-op HUD, effects, display modes, and capture diagnostics.

#ifndef COOP_EXIT_CAPTURE_RUNTIME_H
#define COOP_EXIT_CAPTURE_RUNTIME_H
#include "coop_capture_pixels.h"

struct CoOpExitImageResult {
    const char* source;
    const char* error;
    HRESULT result;
    unsigned width,height,bits;
    // Initialize co-op exit image result with no active retail ownership.
    CoOpExitImageResult() : source("none"),error("surface unavailable"),result(0),width(0),height(0),bits(0) {}
};

// Prepare exit screenshot surface for the owning viewport.
static bool CoOp_ExitScreenshotSurface(const char* path,IDirectDrawSurface7* surface,
    CoOpExitImageResult& result) {
    if(!surface)return false;
    DDSURFACEDESC2 desc;memset(&desc,0,sizeof(desc));desc.dwSize=sizeof(desc);
    result.result=surface->GetSurfaceDesc(&desc);
    if(FAILED(result.result)) {result.error="surface description failed";return false;}
    result.width=desc.dwWidth;result.height=desc.dwHeight;
    result.bits=desc.ddpfPixelFormat.dwRGBBitCount;
    if(!result.width || !result.height || result.width>8192 || result.height>8192 ||
       !(desc.ddpfPixelFormat.dwFlags&DDPF_RGB) ||
       (result.bits!=16 && result.bits!=24 && result.bits!=32)) {
        result.error="unsupported size or RGB format";return false;
    }
    IDirectDrawSurface7* staging=0;
    IDirectDrawSurface7* readable=surface;
    DDSURFACEDESC2 locked;memset(&locked,0,sizeof(locked));locked.dwSize=sizeof(locked);
    result.result=readable->Lock(0,&locked,DDLOCK_READONLY|DDLOCK_DONOTWAIT,0);
    if(FAILED(result.result)) {
        DDSURFACEDESC2 copy;memset(&copy,0,sizeof(copy));copy.dwSize=sizeof(copy);
        copy.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;
        copy.dwWidth=desc.dwWidth;copy.dwHeight=desc.dwHeight;
        copy.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY;
        copy.ddpfPixelFormat=desc.ddpfPixelFormat;
        LPDIRECTDRAW7 draw=0;
        result.result=surface->GetDDInterface(reinterpret_cast<void**>(&draw));
        if(FAILED(result.result) || !draw) {result.error="DirectDraw interface unavailable";return false;}
        result.result=draw->CreateSurface(&copy,&staging,0);
        draw->Release();
        if(FAILED(result.result)) {result.error="staging surface creation failed";return false;}
        result.result=staging->Blt(0,surface,0,DDBLT_ASYNC,0);
        if(SUCCEEDED(result.result)) {
            readable=staging;
            for(unsigned retry=0;retry<20;++retry) {
                result.result=readable->Lock(0,&locked,DDLOCK_READONLY|DDLOCK_DONOTWAIT,0);
                if(SUCCEEDED(result.result))break;
                Sleep(5);
            }
        }
        if(FAILED(result.result)) {
            result.error="surface copy or lock failed";staging->Release();return false;
        }
    }
    bool okay=false;
    unsigned rowBytes=(result.width*3+3)&~3u;
    unsigned char* row=(unsigned char*)malloc(rowBytes);
    if(!row) result.error="scanline allocation failed";
    else {
        FILE* file=fopen(path,"wb");
        if(!file) result.error="BMP file open failed";
        else {
            BITMAPFILEHEADER header;memset(&header,0,sizeof(header));
            BITMAPINFOHEADER info;memset(&info,0,sizeof(info));
            header.bfType=0x4D42;header.bfOffBits=sizeof(header)+sizeof(info);
            header.bfSize=header.bfOffBits+rowBytes*result.height;
            info.biSize=sizeof(info);info.biWidth=result.width;info.biHeight=result.height;
            info.biPlanes=1;info.biBitCount=24;info.biCompression=BI_RGB;
            info.biSizeImage=rowBytes*result.height;
            okay=fwrite(&header,1,sizeof(header),file)==sizeof(header) &&
                fwrite(&info,1,sizeof(info),file)==sizeof(info);
            unsigned red=desc.ddpfPixelFormat.dwRBitMask;
            unsigned green=desc.ddpfPixelFormat.dwGBitMask;
            unsigned blue=desc.ddpfPixelFormat.dwBBitMask;
            if(!red || !green || !blue) {
                red=0x00FF0000;green=0x0000FF00;blue=0x000000FF;
            }
            for(int y=(int)result.height-1;okay && y>=0;--y) {
                memset(row,0,rowBytes);
                const unsigned char* pixels=(const unsigned char*)locked.lpSurface+y*locked.lPitch;
                CoOp_CaptureBgrRow(pixels,row,result.width,result.bits,red,green,blue);
                okay=fwrite(row,1,rowBytes,file)==rowBytes;
            }
            if(fclose(file))okay=false;
            if(!okay) {result.error="BMP write failed";remove(path);}
        }
        free(row);
    }
    readable->Unlock(0);
    if(staging)staging->Release();
    if(okay)result.error="ok";
    return okay;
}
// Prepare exit player status for the owning viewport.
static void CoOp_ExitPlayerStatus(FILE* file,const char* label,CPlayer* player) {
    if(!player) {fprintf(file,"%s: absent\n",label);return;}
    fprintf(file,"%s: ptr=%p hp=%d/%d pos-raw=%ld,%ld,%ld pos-world=%.3f,%.3f,%.3f velocity=%ld,%ld,%ld angles=%d,%d,%d state=%ld flags=%04x collision=%08lx native=%u route=%p\n",
        label,player,player->mHealth,player->mMaxHealth,
        player->mPos.vx,player->mPos.vy,player->mPos.vz,
        player->mPos.vx/4096.0,player->mPos.vy/4096.0,player->mPos.vz/4096.0,
        player->mVel.vx,player->mVel.vy,player->mVel.vz,
        player->mAngles.vx,player->mAngles.vy,player->mAngles.vz,
        player->field_E1C,player->mFlags,player->mCollision,
        (unsigned)(u8)player->field_1AC,
        *reinterpret_cast<void**>(reinterpret_cast<u8*>(player)+0x1B8));
    const u8* body=reinterpret_cast<const u8*>(player);
    const i16* normal=reinterpret_cast<const i16*>(body+0xA8);
    fprintf(file,"%s-entry: animation=%u frame=%d fraction=%d target=%d mode=%u direction=%d finished=%u surface-normal=%d,%d,%d attached=%u floor-initialized=%u camera-initialized=%u look-path=%p swing-rope=%p\n",
        label,(unsigned)player->mAnim,(int)player->mFrame,(int)player->mFrameFrac,
        (int)player->mTargetFrame,(unsigned)player->mAnimMode,(int)player->mAnimDir,
        (unsigned)player->mAnimFinished,normal[0],normal[1],normal[2],
        (unsigned)body[0xAD4],(unsigned)body[0x553],(unsigned)body[0x53C],
        *reinterpret_cast<void* const*>(body+0xC90),
        *reinterpret_cast<void* const*>(body+0xE64));
}
// Prepare exit camera status for the owning viewport.
static void CoOp_ExitCameraStatus(FILE* file,const char* label,CCamera* camera) {
    if(!camera) {fprintf(file,"%s: absent\n",label);return;}
    fprintf(file,"%s: ptr=%p mode=%d tripod=%p focus=%p yaw=%d enabled=%u\n",
        label,camera,(int)camera->mCameraMode,camera->mTripod,
        camera->field_13C,camera->field_23A,camera->field_F8);
}
// Store save exit capture in the owning viewport without changing the shared world.
static void CoOp_SaveExitCapture() {
    char module[MAX_PATH];DWORD length=GetModuleFileNameA(0,module,sizeof(module));
    if(!length || length>=sizeof(module))return;
    char* slash=strrchr(module,'\\');if(!slash)return;
    SYSTEMTIME now;GetLocalTime(&now);
    char stem[64];sprintf(stem,"exit-%04u%02u%02u-%02u%02u%02u-%08lX",
        now.wYear,now.wMonth,now.wDay,now.wHour,now.wMinute,now.wSecond,GetTickCount());
    unsigned dir=(unsigned)(slash-module+1);
    if(dir+strlen(stem)+5>=MAX_PATH)return;
    char image[MAX_PATH],status[MAX_PATH];
    memcpy(image,module,dir);image[dir]=0;strcat(image,stem);strcat(image,".bmp");
    memcpy(status,module,dir);status[dir]=0;strcat(status,stem);strcat(status,".txt");
    CoOpExitImageResult result;
    bool saved=false;
    LPDIRECT3DDEVICE7 device=*reinterpret_cast<LPDIRECT3DDEVICE7*>(0x006B791C);
    LPDIRECTDRAWSURFACE7 target=0;
    result.source="live Direct3D render target";
    if(device)result.result=device->GetRenderTarget(&target);
    if(target) {
        saved=CoOp_ExitScreenshotSurface(image,target,result);
        target->Release();
    } else result.error="render target unavailable";
    FILE* file=fopen(status,"w");if(!file)return;
    fprintf(file,"End-key capture %04u-%02u-%02u %02u:%02u:%02u.%03u\n",
        now.wYear,now.wMonth,now.wDay,now.wHour,now.wMinute,now.wSecond,now.wMilliseconds);
    fprintf(file,"screenshot: %s source=%s file=%s size=%ux%u bits=%u reason=%s HRESULT=%08lX\n",
        saved?"saved":"failed",result.source,saved?image:"none",
        result.width,result.height,result.bits,result.error,(unsigned long)result.result);
    fprintf(file,"session: enabled=%d pending=%d layout=%d view=%d second-rendered=%d pause=%ld letterbox=%ld hud-offset=%ld resolution=%ldx%ld render-device=%p\n",
        (int)gCoOpSession.enabled,(int)gCoOpSession.pending,(int)gCoOpLayout,
        (int)gCoOpViewActive,(int)gCoOpSecondRendered,
        *reinterpret_cast<i32*>(0x00ADB4D8),*reinterpret_cast<i32*>(0x00660F80),
        *reinterpret_cast<i32*>(0x0060F76C),
        *reinterpret_cast<i32*>(0x00568154),*reinterpret_cast<i32*>(0x00568158),device);
    fprintf(file,"checkpoint=%ld level-status=%ld script-bank=%p cinematic-owner=%u script-owner=%u script-active=%d script-ticks=%lu\n",
        *reinterpret_cast<i32*>(0x0055804C),gLevelStatus,
        *reinterpret_cast<void**>(0x006B466C),gCoOpCinematicOwner.slot,
        gCoOpScriptMotion.owner.slot,(int)gCoOpScriptMotion.active,gCoOpScriptMotion.ticks);
    fprintf(file,"life: P1 waiting=%d recover=%d revive=%d P2 waiting=%d recover=%d revive=%d\n",
        (int)gCoOpLife.slot[0].waiting,(int)gCoOpLife.slot[0].recover,(int)gCoOpLife.slot[0].revive,
        (int)gCoOpLife.slot[1].waiting,(int)gCoOpLife.slot[1].recover,(int)gCoOpLife.slot[1].revive);
    fprintf(file,"crane-auto: exits=%u last-owner=%u arrival=%u pending=%u requested=%u skip-flag=%u policy=authored-route-dispatch-and-native-handoff\n",
        gCoOpCraneAutoCount,gCoOpCraneAutoOwner,gCoOpCraneArrivalCount,
        gCoOpCraneArrivalPending,gCoOpCraneArrivalRequested,
        *reinterpret_cast<u8*>(0x0060CFC4));
    CoOp_ExitPlayerStatus(file,"P1",gCoOpPlayers.Get(COOP_PLAYER_ONE));
    CoOp_ExitPlayerStatus(file,"P2",gCoOpPlayers.Get(COOP_PLAYER_TWO));
    for(unsigned slot=0;slot<2;++slot) {
        const CoOpGrabLease& grab=gCoOpGrab[slot];
        CBaddy* enemy=grab.state?CoOp_Grabber(grab):0;
        CPlayer* player=gCoOpPlayers.Get((CoOpPlayerSlot)slot);
        fprintf(file,"P%u-grab: state=%08lx phase=%08lx owned-enemy-flags=%08lx generation=%I64u valid-victim=%u enemy=%p saved-callback=%08lx live-callback=%08lx\n",
            slot+1,grab.state,grab.phase,grab.enemyHoldFlags,grab.victim.generation,
            gCoOpPlayers.Router().Resolve(grab.victim)!=0,enemy,grab.callback,
            player?*reinterpret_cast<u32*>(reinterpret_cast<u8*>(player)+0x554):0);
    }
    CItem* enemyItem=*reinterpret_cast<CItem**>(0x0056E990);
    for(unsigned scan=0;enemyItem && scan<256;++scan,enemyItem=enemyItem->mNextItem) {
        if(enemyItem->mType!=313)continue;
        CBaddy* boss=static_cast<CBaddy*>(enemyItem);
        fprintf(file,"venom-state: ptr=%p health=%d phase=%08lx substate=%ld combat-flags=%08lx hold-damage-block=%u flags=%04x radius=%u animation=%u frame=%d fraction=%d target=%d mode=%u finished=%u\n",
            boss,boss->mHealth,(u32)boss->field_31C.bothFlags,boss->dumbAssPad,
            (u32)boss->field_218,(boss->field_218&0x4000)!=0,boss->mFlags,boss->mRMinor,
            (unsigned)boss->mAnim,(int)boss->mFrame,(int)boss->mFrameFrac,
            (int)boss->mTargetFrame,(unsigned)boss->mAnimMode,(unsigned)boss->mAnimFinished);
    }
    CoOp_RhinoStatus(file);
    CoOp_ArmorStatus(file);
    CoOp_CameraPolicyStatus(file);
    fprintf(file,"lizman-targets: P1=%u P2=%u\n",gCoOpLizManTargets[0],gCoOpLizManTargets[1]);
    fprintf(file,"projectile-collision: installed=%u hits=%u,%u\n",gCoOpProjectileCollisionInstalled,gCoOpProjectileHits[0],gCoOpProjectileHits[1]);
    CoOp_ExitCameraStatus(file,"P1 camera",*reinterpret_cast<CCamera**>(0x0056F3B8));
    CoOp_ExitCameraStatus(file,"P2 camera",gCoOpPlayers.SecondaryCamera());
    fprintf(file,"P2 input: x=%d z=%d held=%08x pressed=%08x\n",
        gCoOpInput.x,gCoOpInput.z,gCoOpInput.held,gCoOpInput.pressed);
    for(int i=0;i<40;++i) {
        const u8* resource=reinterpret_cast<const u8*>(0x006B2440)+i*0x44;
        if(resource[0])fprintf(file,"resource %d %.9s usable=%u\n",i,(const char*)resource,resource[10]);
    }
    fclose(file);
}
#endif
