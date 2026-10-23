#include <assert.h>
#include <stdio.h>
#include "coop/input/coop_controller_route.h"
int main() {
    CoOpControllerRoute r=CoOp_ChooseControllerRoute(0,1,true,false);
    assert(r.p2Slot==-1 && r.p2Keyboard && !r.reserveNativeController);
    r=CoOp_ChooseControllerRoute(1u<<0,1,true,false);
    assert(r.p2Slot==-1 && r.p2Keyboard && !r.reserveNativeController);
    r=CoOp_ChooseControllerRoute(1u<<0,2,true,false);
    assert(r.p2Slot==0 && !r.p2Keyboard && r.reserveNativeController);
    r=CoOp_ChooseControllerRoute(1u<<3,2,true,false);
    assert(r.p2Slot==3 && !r.p2Keyboard && r.reserveNativeController);
    r=CoOp_ChooseControllerRoute((1u<<0)|(1u<<1),2,true,false);
    assert(r.p2Slot==1 && !r.p2Keyboard && !r.reserveNativeController);
    r=CoOp_ChooseControllerRoute((1u<<0)|(1u<<2),1,true,false);
    assert(r.p2Slot==2 && !r.p2Keyboard && !r.reserveNativeController);
    r=CoOp_ChooseControllerRoute((1u<<1)|(1u<<3),2,true,false);
    assert(r.p2Slot==3 && !r.p2Keyboard && !r.reserveNativeController);
    r=CoOp_ChooseControllerRoute(15,1,true,false);
    assert(r.p2Slot==1 && !r.p2Keyboard && !r.reserveNativeController);
    assert(CoOp_BindingCaptureSlot(0)==-1);
    assert(CoOp_BindingCaptureSlot(1u<<3)==3);
    assert(CoOp_BindingCaptureSlot((1u<<0)|(1u<<2))==2);
    r=CoOp_ChooseControllerRoute(1,2,true,true);
    assert(r.p2Slot==-1 && !r.reserveNativeController);
    r=CoOp_ChooseControllerRoute(1,2,false,false);
    assert(r.p2Slot==-1 && !r.reserveNativeController);
    puts("PASS automatic pad detection: single-pad P1/P2, sparse two-pad routing, binding capture, menu state");
    return 0;
}
