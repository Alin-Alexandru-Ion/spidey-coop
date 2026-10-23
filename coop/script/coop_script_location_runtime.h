// Co-op / script: Coordinate authored motion and cinematics with co-op control handoffs.

#ifndef COOP_SCRIPT_LOCATION_RUNTIME_H
#define COOP_SCRIPT_LOCATION_RUNTIME_H

// Manual single-player probes for authored movement. Coordinates are the
// native 12-bit fixed-point CPlayer position, with readable world units too.
static bool gCoOpLocationStartDown,gCoOpLocationEndDown,gCoOpLocationHasStart;
static unsigned gCoOpLocationNextPair,gCoOpLocationPair;
static CVector gCoOpLocationStart;
static char gCoOpLocationStartLevel[16];

// Track location level until the authored control handoff completes.
static void CoOp_LocationLevel(char level[16]) {
    strcpy(level,"unknown");
    for(int i=0;i<40;++i) {
        const u8* resource=reinterpret_cast<const u8*>(0x006B2440)+i*0x44;
        if(!resource[10])continue;
        char name[10];memcpy(name,resource,9);name[9]=0;
        if(strstr(name,"_G") || strstr(name,"_g")) {
            strcpy(level,name);return;
        }
    }
}
// Store write script location in the authored scene and its recorded player owner.
static void CoOp_WriteScriptLocation(bool start,CPlayer* player) {
    char path[MAX_PATH];
    DWORD length=GetModuleFileNameA(0,path,sizeof(path));
    if(!length || length>=sizeof(path))return;
    char* slash=strrchr(path,'\\');
    if(!slash || slash-path+1+strlen("script-locations.log")>=sizeof(path))return;
    strcpy(slash+1,"script-locations.log");
    FILE* log=fopen(path,"a");if(!log)return;
    SYSTEMTIME now;GetLocalTime(&now);
    char level[16];CoOp_LocationLevel(level);
    const CVector position=player->mPos;
    if(start) {
        gCoOpLocationStart=position;
        strcpy(gCoOpLocationStartLevel,level);
        gCoOpLocationPair=++gCoOpLocationNextPair;
        gCoOpLocationHasStart=true;
    }
    const unsigned pair=gCoOpLocationHasStart?gCoOpLocationPair:0;
    CCamera* camera=*reinterpret_cast<CCamera**>(0x0056F3B8);
    fprintf(log,"%04u-%02u-%02u %02u:%02u:%02u.%03u pair=%u %s level=%s raw=(%ld,%ld,%ld) world=(%.3f,%.3f,%.3f) yaw=%d hp=%d native=%u route=%p camera=%d letterbox=%ld\n",
        now.wYear,now.wMonth,now.wDay,now.wHour,now.wMinute,now.wSecond,now.wMilliseconds,
        pair,start?"START":"END",level,
        position.vx,position.vy,position.vz,
        position.vx/4096.0,position.vy/4096.0,position.vz/4096.0,
        player->mAngles.vy,player->mHealth,(unsigned)(u8)player->field_1AC,
        *reinterpret_cast<void**>(reinterpret_cast<u8*>(player)+0x1B8),
        camera?(int)camera->mCameraMode:-1,*reinterpret_cast<i32*>(0x00660F80));
    if(!start && gCoOpLocationHasStart) {
        fprintf(log,"pair=%u DELTA start-level=%s end-level=%s raw=(%ld,%ld,%ld) world=(%.3f,%.3f,%.3f)\n",
            pair,gCoOpLocationStartLevel,level,
            position.vx-gCoOpLocationStart.vx,position.vy-gCoOpLocationStart.vy,
            position.vz-gCoOpLocationStart.vz,
            (position.vx-gCoOpLocationStart.vx)/4096.0,
            (position.vy-gCoOpLocationStart.vy)/4096.0,
            (position.vz-gCoOpLocationStart.vz)/4096.0);
        gCoOpLocationHasStart=false;
    }
    fclose(log);
}
// Store capture script location keys in the authored scene and its recorded player owner.
static void CoOp_CaptureScriptLocationKeys(bool startDown,bool endDown) {
    const bool startEdge=startDown && !gCoOpLocationStartDown;
    const bool endEdge=endDown && !gCoOpLocationEndDown;
    gCoOpLocationStartDown=startDown;gCoOpLocationEndDown=endDown;
    if(!startEdge && !endEdge)return;
    if(gCoOpSession.enabled || gCoOpSession.pending ||
       gCoOpPlayers.Get(COOP_PLAYER_TWO) ||
       *reinterpret_cast<i32*>(0x00ADB4D8))return;
    CPlayer* player=gCoOpPlayers.Get(COOP_PLAYER_ONE);
    if(!player)return;
    if(startEdge)CoOp_WriteScriptLocation(true,player);
    if(endEdge)CoOp_WriteScriptLocation(false,player);
}

#endif
