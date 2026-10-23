// Co-op / input: Route keyboard/gamepad input and settings to the intended player.
#pragma once
#ifndef COOP_CONFIG_H
#define COOP_CONFIG_H
#include "coop_gamepad.h"
// DirectInput scan codes, matching the original PC keyboard path.
struct CoOpConfig {
    unsigned char keys[11]; // left, right, forward, back, then seven CoOpActions
    unsigned char pad[11];
    unsigned layout;
    unsigned singleControllerOwner; // the only pad goes to P1 or P2
    // Initialize co-op config with no active retail ownership.
    CoOpConfig() { Defaults(); }
    // Translate defaults into the selected player's controls.
    void Defaults() {
        const unsigned char initial[11]={0xCB,0xCD,0xC8,0xD0,0x4F,0x50,0x51,0x52,0x4B,0x4C,0x4D};
        for(unsigned i=0;i<11;++i) keys[i]=initial[i];
        const unsigned char initialPad[11]={COOP_PAD_LS_LEFT,COOP_PAD_LS_RIGHT,
            COOP_PAD_LS_UP,COOP_PAD_LS_DOWN,COOP_PAD_A,COOP_PAD_X,
            COOP_PAD_B,COOP_PAD_RT,COOP_PAD_LT,COOP_PAD_Y,COOP_PAD_RB};
        for(i=0;i<11;++i) pad[i]=initialPad[i];
        layout=1;
        singleControllerOwner=1;
    }
    // Report the current allowed state of the selected player's bindings and device state.
    static bool Allowed(unsigned key) {
        return key>1 && key<256 && key!=0x1C && key!=0x9C &&
            key!=0xCF && key!=0x33;
    }
    // Report whether uses is true for the selected player's bindings and device state.
    bool Uses(unsigned key) const {
        for(unsigned i=0;i<11;++i) if(keys[i]==key) return true;
        return false;
    }
    // Translate bind into the selected player's controls.
    bool Bind(unsigned action,unsigned key) {
        if(action>=11 || !Allowed(key)) return false;
        // A duplicate swaps bindings so no action becomes inaccessible.
        for(unsigned i=0;i<11;++i) if(i!=action && keys[i]==key) keys[i]=keys[action];
        keys[action]=(unsigned char)key;return true;
    }
    // Store bind pad in the selected player's bindings and device state.
    bool BindPad(unsigned action,unsigned code) {
        if(action>=11 || !CoOp_PadMask(code)) return false;
        for(unsigned i=0;i<11;++i) if(i!=action && pad[i]==code) pad[i]=pad[action];
        pad[action]=(unsigned char)code;return true;
    }
};
#endif
