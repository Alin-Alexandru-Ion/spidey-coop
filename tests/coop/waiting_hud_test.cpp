#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef unsigned char u8;typedef unsigned short u16;typedef unsigned long u32;typedef long i32;
#define D3DCLEAR_TARGET 1
struct Font {const char* field_38;void* pCharTab;u32 mRed,mGreen,mBlue;};
struct CoOpHudPane {float x,y,w,h;};
static Font font={"saved",0},face={"sp_fnt02.fnt",(void*)1},*fonts[6]={&face};
static u8 textState[8]={1,2,3,4,5,6,7,8};static u32 logicalWidth=640,logicalHeight=360;
static int calls,expectedX,expectedY;static const char* expectedPlayer;
struct Device {int clears;void Clear(int n,void* r,int flags,u32 color,float z,int stencil){assert(n==0 && !r && flags==D3DCLEAR_TARGET && color==0xFF000000);++clears;}};
typedef Device* LPDIRECT3DDEVICE7;
static i32 __cdecl Draw(i32 x,i32 y,const char* s,i32,u32){assert(font.mRed==255 && font.mGreen==255 && font.mBlue==255);assert(x==expectedX);assert(y==expectedY+(calls%2?2:-12));assert(!strcmp(s,calls%2?"Waiting for checkpoint respawn":expectedPlayer));++calls;return 0;}
#include "waiting_hud_runtime_test.h"
int main(){Device d;d.clears=0;u8 saved[8];memcpy(saved,textState,8);
    for(int horizontal=0;horizontal<2;++horizontal)for(unsigned owner=1;owner<=2;++owner){
        CoOpHudPane pane={0,0,1920,1080};
        if(horizontal){pane.h=540;pane.y=(owner-1)*540;}else{pane.w=960;pane.x=(owner-1)*960;}
        expectedX=(int)((pane.x+pane.w/2)*640/1920);expectedY=(int)((pane.y+pane.h/2)*360/1080);
        expectedPlayer=owner==1?"P1":"P2";CoOp_HudWaitingPane(&d,pane,owner,1920,1080);
        assert(!strcmp(font.field_38,"saved") && !memcmp(saved,textState,8));
    }
    assert(calls==8 && d.clears==4);fonts[0]=0;CoOpHudPane pane={0,0,960,1080};CoOp_HudWaitingPane(&d,pane,1,1920,1080);assert(calls==8 && d.clears==5);
    puts("PASS waiting HUD: both owners/layouts, logical-resolution coordinates, black viewport clear, font/state restoration and missing-font fallback");return 0;
}
