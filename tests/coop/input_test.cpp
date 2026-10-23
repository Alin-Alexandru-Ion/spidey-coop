#include <assert.h>
#include <stdio.h>
#include "coop/input/coop_input.h"
static void __cdecl Button(SButton* b,int v) {b->Pressed=v!=0;}
int main() {
    SControl c;CoOp_ClearControl(c);
    assert(c.pTriangle==&c.LeftOne && c.pX==&c.RightTwo);
    CoOpInput in;in.held=COOP_JUMP;CoOp_FillControl(c,in,Button);
    assert(c.X.Pressed && c.Crouch.Pressed && !c.Triangle.Pressed && !c.Jump.Pressed);
    in.held=COOP_WEB;CoOp_FillControl(c,in,Button);
    assert(c.Triangle.Pressed && c.Jump.Pressed && !c.X.Pressed && !c.Crouch.Pressed);
    in.x=-1;in.z=1;
    in.held=COOP_JUMP|COOP_PUNCH|COOP_KICK|COOP_WEB|COOP_AIM|COOP_ZIP|COOP_SWING;
    CoOp_FillControl(c,in,Button);
    assert(c.Left.Pressed && c.Up.Pressed && !c.Right.Pressed && !c.Down.Pressed);
    assert(c.Triangle.Pressed && c.Jump.Pressed && c.Square.Pressed && c.SmartBomb.Pressed);
    assert(c.Circle.Pressed && c.SelectWeapon.Pressed && c.X.Pressed && c.Crouch.Pressed);
    assert(c.LeftOne.Pressed && c.RightOne.Pressed && c.RightTwo.Pressed);
    assert(!c.Start.Pressed && !c.Select.Pressed);
    in=CoOpInput();CoOp_FillControl(c,in,Button);
    for(int i=0;i<20;++i) assert(!(&c.Triangle)[i].Pressed);
    puts("PASS: full P2 digital action mapping, physical/gameplay aliases, release, no pause binding");
    return 0;
}
