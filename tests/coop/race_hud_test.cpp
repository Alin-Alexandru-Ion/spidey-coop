#include <assert.h>
#include <stdio.h>
typedef unsigned long u32;
enum {HUD_SHARED=0,HUD_RACE=6};
static struct Capture {bool active;unsigned owner,style;} gHudCapture;
static bool enabled;
static unsigned calls,lastOwner,lastStyle;
static bool CoOp_HudEnabled(){return enabled;}
class CoOpHudScope {
    Capture saved;
public:
    CoOpHudScope(unsigned owner,unsigned style):saved(gHudCapture) {
        gHudCapture.active=true;gHudCapture.owner=owner;gHudCapture.style=style;
    }
    ~CoOpHudScope(){gHudCapture=saved;}
};
static void __cdecl NativeProgress(const u32*,u32*) {
    ++calls;lastOwner=gHudCapture.owner;lastStyle=gHudCapture.style;
}
#include "race_hud_runtime_test.h"
int main() {
    enabled=false;gHudCapture.active=true;gHudCapture.owner=1;gHudCapture.style=HUD_SHARED;
    CoOp_HudRaceProgress(reinterpret_cast<u32>(&NativeProgress),0,0);
    assert(calls==1 && lastOwner==1 && lastStyle==HUD_SHARED);
    enabled=true;gHudCapture.active=false;
    CoOp_HudRaceProgress(reinterpret_cast<u32>(&NativeProgress),0,0);
    assert(calls==2 && lastOwner==1 && lastStyle==HUD_SHARED && !gHudCapture.active);
    gHudCapture.active=true;
    CoOp_HudRaceProgress(reinterpret_cast<u32>(&NativeProgress),0,0);
    assert(calls==3 && lastOwner==0 && lastStyle==HUD_RACE);
    assert(gHudCapture.active && gHudCapture.owner==1 && gHudCapture.style==HUD_SHARED);
    puts("PASS race callback: shared capture in co-op, retail path elsewhere, scope restored");
    return 0;
}
