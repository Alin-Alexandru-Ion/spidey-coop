#include <windows.h>
#include <assert.h>
#include <stdio.h>
typedef unsigned long u32;
typedef unsigned char u8;
enum {HUD_COMPASS=5};
struct Capture {bool active;unsigned owner,style;};
static Capture gHudCapture;
static unsigned gHudCompassCount[2],nativeCalls,observedOwner[2];
static int cameraX,observedCamera[2];
static u8 compassStatus;
static bool CoOp_HudEnabled(){return true;}
class CoOpHudScope {
    Capture saved;
public:
    CoOpHudScope(unsigned owner,unsigned style):saved(gHudCapture){
        gHudCapture.active=true;gHudCapture.owner=owner;gHudCapture.style=style;
    }
    ~CoOpHudScope(){gHudCapture=saved;}
};
static void __cdecl NativeCompass(){
    assert(nativeCalls<2);
    assert(gHudCapture.style==HUD_COMPASS);
    observedOwner[nativeCalls]=gHudCapture.owner;
    observedCamera[nativeCalls]=cameraX;
    ++nativeCalls;
}
#include "compass_runtime_test.h"
int main(){
    compassStatus=1;
    gHudCapture.active=true;gHudCapture.owner=1;
    CoOp_HudCompass();assert(nativeCalls==0); // native panel branch is deferred
    gHudCapture.active=false;
    cameraX=11;CoOp_HudPersonalCompass(1);
    cameraX=29;CoOp_HudPersonalCompass(2);
    assert(nativeCalls==2 && observedOwner[0]==1 && observedOwner[1]==2);
    assert(observedCamera[0]==11 && observedCamera[1]==29);
    assert(gHudCompassCount[0]==1 && gHudCompassCount[1]==1 && !gHudCapture.active);
    compassStatus=0;CoOp_HudPersonalCompass(1);assert(nativeCalls==2);
    puts("PASS compass: one native direction calculation per player using separate camera states and shared objective");
    return 0;
}
