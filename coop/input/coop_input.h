// Co-op / input: Route keyboard/gamepad input and settings to the intended player.
#pragma once
#ifndef COOP_INPUT_H
#define COOP_INPUT_H
#include <string.h>
#include "ps2pad.h"
enum CoOpAction {COOP_JUMP=1,COOP_PUNCH=2,COOP_KICK=4,COOP_SWING=8,
    COOP_AIM=16,COOP_WEB=32,COOP_ZIP=64};
struct CoOpInput
{
    int x,z;
    unsigned int held,pressed;
    // Initialize co-op input with no active retail ownership.
    CoOpInput() : x(0),z(0),held(0),pressed(0) {}
};
// Match retail PC Pad_Update's physical and gameplay aliases. The callback is
// retail Pad_Button, so hold, release, trigger and double-tap timing remain retail.
static void CoOp_FillControl(SControl& c,const CoOpInput& input,
    // Translate void into the selected player's controls.
    void (__cdecl *button)(SButton*,int))
{
    int held[20]={0};
    held[3]=held[16]=(input.held&COOP_JUMP)!=0; // native X / PC alias
    held[1]=held[18]=(input.held&COOP_PUNCH)!=0;
    held[2]=held[19]=(input.held&COOP_KICK)!=0;
    held[0]=held[17]=(input.held&COOP_WEB)!=0; // native Triangle / PC alias
    held[4]=(input.held&COOP_AIM)!=0;
    held[6]=(input.held&COOP_ZIP)!=0;
    held[7]=(input.held&COOP_SWING)!=0;
    held[8]=input.x<0;held[9]=input.x>0;
    held[10]=input.z>0;held[11]=input.z<0;
    SButton* first=&c.Triangle;
    for(int i=0;i<20;++i) {
        first[i].Triggered=0;first[i].field_2=0;
        button(first+i,held[i]);
    }
}
// Reset every P2 action edge before a new control frame.
static void CoOp_ClearControl(SControl& c)
{
    memset(&c,0,sizeof(c));
    c.Type=65;
    c.RawAnalogueMoveForwardsBackwards=c.RawAnalogueMoveLeftRight=128;
    c.RawAnalogueAimForwardsBackwards=c.RawAnalogueAimLeftRight=128;
    c.pTriangle=&c.LeftOne;c.pSquare=&c.LeftTwo;
    c.pCircle=&c.RightOne;c.pX=&c.RightTwo;
}
#endif
