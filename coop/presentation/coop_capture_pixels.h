// Co-op / presentation: Render co-op HUD, effects, display modes, and capture diagnostics.

#ifndef COOP_CAPTURE_PIXELS_H
#define COOP_CAPTURE_PIXELS_H

// Convert the game's 16/24/32-bit RGB DirectDraw scanline to BMP BGR bytes.
inline unsigned char CoOp_CaptureChannel(unsigned pixel,unsigned mask) {
    if(!mask)return 0;
    unsigned shift=0;
    while(!(mask&1)) {mask>>=1;++shift;}
    unsigned value=(pixel>>shift)&mask;
    return (unsigned char)((value*255u+mask/2)/mask);
}
// Store capture bgr row in the owning viewport without changing the shared world.
inline void CoOp_CaptureBgrRow(const unsigned char* source,unsigned char* dest,
    unsigned width,unsigned bits,unsigned red,unsigned green,unsigned blue) {
    const unsigned bytes=bits/8;
    for(unsigned x=0;x<width;++x) {
        unsigned pixel=0;
        for(unsigned c=0;c<bytes;++c)
            pixel|=(unsigned)source[x*bytes+c]<<(c*8);
        dest[x*3]=CoOp_CaptureChannel(pixel,blue);
        dest[x*3+1]=CoOp_CaptureChannel(pixel,green);
        dest[x*3+2]=CoOp_CaptureChannel(pixel,red);
    }
}

#endif
