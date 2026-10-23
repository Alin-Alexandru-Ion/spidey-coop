#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef unsigned char u8;
typedef unsigned long u32;
typedef long i32;
#include "coop/input/coop_gamepad_runtime.h"
static CoOpXInputState sample;
static unsigned lastSlot;
static bool disconnected;
static unsigned long WINAPI FakeState(unsigned long slot,CoOpXInputState* out) {
    lastSlot=slot;
    if(disconnected) return 1167;
    *out=sample;return 0;
}
int main() {
    gCoOpXInputChecked=true;gCoOpXInputGetState=FakeState;
    bool connected=false;
    assert(CoOp_ReadGamepad(4,&connected)==0 && !connected);
    memset(&sample,0,sizeof(sample));
    sample.pad.buttons=0x1000|0x4000|0x0100;
    sample.pad.leftTrigger=31;sample.pad.rightTrigger=30;
    sample.pad.lx=-12001;sample.pad.ly=12001;
    sample.pad.rx=12000;sample.pad.ry=-12001;
    unsigned mask=CoOp_ReadGamepad(1,&connected);
    assert(connected && lastSlot==1);
    assert(CoOp_PadDown(mask,COOP_PAD_A) && CoOp_PadDown(mask,COOP_PAD_X));
    assert(CoOp_PadDown(mask,COOP_PAD_LB) && CoOp_PadDown(mask,COOP_PAD_LT));
    assert(CoOp_PadDown(mask,COOP_PAD_LS_LEFT) && CoOp_PadDown(mask,COOP_PAD_LS_UP));
    assert(CoOp_PadDown(mask,COOP_PAD_RS_DOWN));
    assert(!CoOp_PadDown(mask,COOP_PAD_RT) && !CoOp_PadDown(mask,COOP_PAD_RS_RIGHT));
    disconnected=true;
    assert(CoOp_ReadGamepad(1,&connected)==0 && !connected);
    puts("PASS XInput masks, thresholds, slot selection and disconnect");
    return 0;
}
