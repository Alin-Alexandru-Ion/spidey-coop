#include <assert.h>
#include <stdio.h>
enum ECameraMode {CAMERAMODE_DEMO=3,CAMERAMODE_FAR=5,CAMERAMODE_USER=16};
struct CCamera {ECameraMode mCameraMode;unsigned char field_F9;};
struct CItem {int mType;CItem* mNextItem;};
struct CBaddy:CItem {int mHealth;};
static CCamera primary={CAMERAMODE_USER},secondary={CAMERAMODE_DEMO};
static CCamera* retailCamera=&primary;
static CItem* baddies;
static struct {bool enabled;} gCoOpSession={true};
static struct Registry {CCamera* camera;CCamera* SecondaryCamera(){return camera;}} gCoOpPlayers={&secondary};
#include "boss_camera_runtime_test.h"
int main() {
    CBaddy boss;boss.mType=310;boss.mNextItem=0;boss.mHealth=900;
    baddies=&boss;
    primary.field_F9=1;
    CoOp_SyncBossCamera();assert(secondary.mCameraMode==CAMERAMODE_USER);
    assert(secondary.field_F9==1);
    primary.field_F9=0;CoOp_SyncBossCamera();assert(secondary.field_F9==0);
    primary.mCameraMode=CAMERAMODE_FAR;CoOp_SyncBossCamera();
    assert(secondary.mCameraMode==CAMERAMODE_DEMO);
    primary.mCameraMode=CAMERAMODE_USER;CoOp_SyncBossCamera();
    assert(secondary.mCameraMode==CAMERAMODE_USER);
    boss.mHealth=0;CoOp_SyncBossCamera();
    assert(secondary.mCameraMode==CAMERAMODE_DEMO);
    boss.mHealth=900;boss.mType=307;CoOp_SyncBossCamera();
    assert(secondary.mCameraMode==CAMERAMODE_USER);
    primary.field_F9=1;CoOp_SyncBossCamera();assert(secondary.field_F9==1);
    // Authored boss cameras outside the Scorpion/Rhino override retain the
    // same camera-specific obstruction feature when both are in USER mode.
    boss.mHealth=0;CoOp_SyncBossCamera();
    secondary.mCameraMode=CAMERAMODE_USER;
    secondary.field_F9=0;CoOp_SyncBossCamera();assert(secondary.field_F9==1);
    gCoOpSession.enabled=false;CoOp_SyncBossCamera();
    assert(secondary.mCameraMode==CAMERAMODE_USER);
    secondary.mCameraMode=CAMERAMODE_DEMO;
    gCoOpSession.enabled=true;gCoOpPlayers.camera=0;CoOp_SyncBossCamera();
    assert(secondary.mCameraMode==CAMERAMODE_DEMO);
    puts("PASS boss camera: P2 USER mode and native obstruction fade enabled for retail boss cameras");
    return 0;
}
