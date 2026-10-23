#include <assert.h>
#include <stdio.h>
#include "coop/presentation/coop_capture_pixels.h"
int main() {
    const unsigned char rgb565[]={0x00,0xF8, 0xE0,0x07, 0x1F,0x00, 0xFF,0xFF};
    unsigned char bgr[12]={0};
    CoOp_CaptureBgrRow(rgb565,bgr,4,16,0xF800,0x07E0,0x001F);
    const unsigned char expected565[]={0,0,255, 0,255,0, 255,0,0, 255,255,255};
    for(int i=0;i<12;++i)assert(bgr[i]==expected565[i]);
    const unsigned char rgb555[]={0x00,0x7C};
    CoOp_CaptureBgrRow(rgb555,bgr,1,16,0x7C00,0x03E0,0x001F);
    assert(bgr[0]==0 && bgr[1]==0 && bgr[2]==255);
    const unsigned char rgb24[]={0x12,0x34,0x56,0xAA,0xBB,0xCC};
    CoOp_CaptureBgrRow(rgb24,bgr,2,24,0xFF0000,0x00FF00,0x0000FF);
    for(i=0;i<6;++i)assert(bgr[i]==rgb24[i]);
    const unsigned char rgb32[]={0x21,0x43,0x65,0xFF};
    CoOp_CaptureBgrRow(rgb32,bgr,1,32,0xFF0000,0x00FF00,0x0000FF);
    assert(bgr[0]==0x21 && bgr[1]==0x43 && bgr[2]==0x65);
    puts("PASS DirectDraw RGB565, RGB555, 24-bit and 32-bit screenshot conversion");
    return 0;
}
