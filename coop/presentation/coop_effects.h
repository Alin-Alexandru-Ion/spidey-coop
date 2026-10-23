// Co-op / presentation: Render co-op HUD, effects, display modes, and capture diagnostics.
#pragma once
#ifndef COOP_EFFECTS_H
#define COOP_EFFECTS_H
// Verified retail Bit_Init registrations. TextBoxList is screen-space/common;
// these callbacks project live world positions and contain no Move dispatch.
inline bool CoOp_WorldEffectSlot(unsigned long list,unsigned long draw) {
    const unsigned long pairs[][2]={
        {0x56EA50,0x40DBD0},{0x56EB40,0x40E840},{0x56E9E0,0x40F110},
        {0x56EB30,0x411EF0},{0x56E9D8,0x4125C0},{0x56EB2C,0x4097E0},
        {0x56E9A0,0x40BAC0},{0x56E9E4,0x40C6F0},{0x56EA34,0x411560},
        {0x56E9CC,0x412F10},{0x56EB34,0x408AF0}};
    for(unsigned i=0;i<11;++i) if(list==pairs[i][0] && draw==pairs[i][1]) return true;
    return false;
}
struct CoOpEffectProjection {
    double focalX,focalY,centerX,centerY;
    // Prepare set for the owning viewport.
    bool Set(const float* projection,int pixelsX,int pixelsY,int logicalX,int logicalY) {
        if(!projection || pixelsX<=0 || pixelsY<=0 || logicalX<=0 || logicalY<=0) return false;
        focalX=projection[0]*logicalX/pixelsX;focalY=projection[5]*logicalY/pixelsY;
        centerX=projection[8]*logicalX/pixelsX;centerY=projection[9]*logicalY/pixelsY;
        return focalX>0 && focalY>0;
    }
    // Return bound from the owning viewport without changing the shared world.
    static long Bound(double n) {if(n>32767) return 32767;if(n< -32767) return -32767;return (long)n;}
    // Prepare remap for the owning viewport.
    void Remap(long& x,long& y,long depth,long oldFocal,long oldWidth,long oldHeight,long basisX,long basisY) const {
        if(!depth || oldFocal<=0 || basisX<=0 || basisY<=0) return; // keep retail's zero-depth sentinel
        x=Bound(centerX+(x-oldWidth/2)*focalX*4096.0/(oldFocal*(double)basisX));
        y=Bound(centerY+(y-oldHeight/2)*focalY*4096.0/(oldFocal*(double)basisY));
    }
};
#endif
