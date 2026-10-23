// Co-op / input: Route keyboard/gamepad input and settings to the intended player.
#pragma once
#ifndef COOP_GAMEPAD_H
#define COOP_GAMEPAD_H

// XInput button bits plus virtual stick/trigger directions. These codes are
// independent of the retail DirectInput controller mapping used by P1.
enum CoOpPadCode {
    COOP_PAD_UP=1,COOP_PAD_DOWN,COOP_PAD_LEFT,COOP_PAD_RIGHT,
    COOP_PAD_START,COOP_PAD_BACK,COOP_PAD_LS_BUTTON,COOP_PAD_RS_BUTTON,
    COOP_PAD_LB,COOP_PAD_RB,COOP_PAD_A,COOP_PAD_B,COOP_PAD_X,COOP_PAD_Y,
    COOP_PAD_LT,COOP_PAD_RT,COOP_PAD_LS_UP,COOP_PAD_LS_DOWN,
    COOP_PAD_LS_LEFT,COOP_PAD_LS_RIGHT,COOP_PAD_RS_UP,COOP_PAD_RS_DOWN,
    COOP_PAD_RS_LEFT,COOP_PAD_RS_RIGHT
};
// Translate pad name into the selected player's controls.
static const char* CoOp_PadName(unsigned code) {
    static const char* names[]={"NONE","DPAD UP","DPAD DOWN","DPAD LEFT","DPAD RIGHT",
        "START","BACK","LEFT STICK CLICK","RIGHT STICK CLICK","LB","RB",
        "A","B","X","Y","LT","RT","LEFT STICK UP","LEFT STICK DOWN",
        "LEFT STICK LEFT","LEFT STICK RIGHT","RIGHT STICK UP","RIGHT STICK DOWN",
        "RIGHT STICK LEFT","RIGHT STICK RIGHT"};
    return code<sizeof(names)/sizeof(names[0])?names[code]:"UNKNOWN";
}
// Translate pad mask into the selected player's controls.
static unsigned CoOp_PadMask(unsigned code) {return code>=1 && code<=24?1u<<(code-1):0;}
// Translate pad down into the selected player's controls.
static int CoOp_PadDown(unsigned mask,unsigned code) {return (mask&CoOp_PadMask(code))!=0;}
#endif
