// Co-op / input: Route keyboard/gamepad input and settings to the intended player.
#pragma once
#ifndef COOP_CONTROLLER_ROUTE_H
#define COOP_CONTROLLER_ROUTE_H

struct CoOpControllerRoute {
    int p2Slot; // -1 means no controller assigned to P2
    bool p2Keyboard;
    bool reserveNativeController;
};

// The retail game has one DirectInput controller. Choose the first two
// connected XInput pads by detection order; only the one-pad case needs a
// player preference. The same selector is used by P2 binding capture.
static int CoOp_ControllerSlot(unsigned connectedSlots,unsigned ordinal) {
    for(unsigned slot=0;slot<4;++slot) if(connectedSlots&(1u<<slot)) {
        if(!ordinal)return (int)slot;
        --ordinal;
    }
    return -1;
}
// Capture binding slot for the selected player's bindings and device state.
static int CoOp_BindingCaptureSlot(unsigned connectedSlots) {
    int second=CoOp_ControllerSlot(connectedSlots,1);
    return second>=0?second:CoOp_ControllerSlot(connectedSlots,0);
}
// Return choose controller route from the selected player's bindings and device state.
static CoOpControllerRoute CoOp_ChooseControllerRoute(unsigned connectedSlots,
    unsigned singleOwner,bool joined,bool optionsOpen) {
    CoOpControllerRoute route={-1,false,false};
    if(!joined || optionsOpen) return route;
    int first=CoOp_ControllerSlot(connectedSlots,0);
    int second=CoOp_ControllerSlot(connectedSlots,1);
    if(second>=0) {
        route.p2Slot=second;
    } else if(first>=0 && singleOwner==2) {
        route.p2Slot=first;
        route.reserveNativeController=true;
    } else {
        route.p2Keyboard=true;
    }
    return route;
}
#endif
