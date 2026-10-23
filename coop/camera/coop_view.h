// Co-op / camera: Select and render independent camera behavior in the shared world.
#pragma once
#ifndef COOP_VIEW_H
#define COOP_VIEW_H

// Layout selects presentation only. COOP_SPLIT_OFF retains its numeric value
// for diagnostics, but means P1 full screen, never player/zone deactivation.
enum CoOpSplitLayout { COOP_SPLIT_OFF, COOP_SPLIT_LEFT_RIGHT, COOP_SPLIT_TOP_BOTTOM,
    COOP_VIEW_PLAYER_TWO, COOP_VIEW_COUNT };
// Cycle only the presentation layout; both players keep updating.
inline CoOpSplitLayout CoOp_NextView(CoOpSplitLayout layout) {
    return static_cast<CoOpSplitLayout>((layout+1)%COOP_VIEW_COUNT);
}
// Report whether this layout displays the owner's camera this frame.
inline bool CoOp_ViewShowsOwner(CoOpSplitLayout layout,unsigned owner) {
    if(!owner) return true;
    if(layout==COOP_SPLIT_OFF) return owner==1;
    if(layout==COOP_VIEW_PLAYER_TWO) return owner==2;
    return true;
}
// Retail M3d_RenderSetup accesses through +0x12. Do not use the incomplete
// decompiled SViewport (0x10 bytes) as a scratch allocation.
struct CoOpRetailViewport
{
    unsigned short right, bottom, left, top;
    unsigned short nearPlane, farPlane, zoom, focalLength, centerX, centerY;
};
// Resize P1's viewport and preserve vertical field of view for split mode.
inline bool CoOp_BuildPrimaryView(const CoOpRetailViewport& full,
    CoOpSplitLayout layout, CoOpRetailViewport& view)
{
    view = full;
        if (full.right <= full.left || full.bottom <= full.top || !full.zoom) return false;
    unsigned long width = full.right - full.left;
    unsigned long height = full.bottom - full.top;
    if (width < 4 || height < 4) return false;
    if (layout == COOP_VIEW_PLAYER_TWO) return true;
    if (layout != COOP_SPLIT_LEFT_RIGHT && layout != COOP_SPLIT_TOP_BOTTOM) return false;
    if (layout == COOP_SPLIT_LEFT_RIGHT) view.right = (unsigned short)(full.left + width / 2);
    else view.bottom = (unsigned short)(full.top + height / 2);
    // Retail zoom is inverse focal length relative to viewport width. Preserve
    // vertical angular FOV: focal pixels scale equally in X/Y with view height.
    double zoom = (double)full.zoom * (view.right - view.left) * height /
        ((double)width * (view.bottom - view.top));
    // A very wide top/bottom pane otherwise exposes extreme peripheral
    // perspective. Increase focal length 35 percent, equally on both axes.
    if (layout == COOP_SPLIT_TOP_BOTTOM) zoom /= 1.35;
    if (zoom < 1.0 || zoom > 65535.0) { view = full; return false; }
    view.zoom = (unsigned short)(zoom + 0.5);
    return true;
}

// Position P2's viewport beside P1 and match its optical scale.
inline bool CoOp_BuildSecondaryView(const CoOpRetailViewport& full,
    CoOpSplitLayout layout, CoOpRetailViewport& view)
{
    if(layout==COOP_VIEW_PLAYER_TWO) return CoOp_BuildPrimaryView(full,layout,view);
    CoOpRetailViewport first;
    if(!CoOp_BuildPrimaryView(full,layout,first)) {view=full;return false;}
    view=full;
    if(layout==COOP_SPLIT_LEFT_RIGHT) view.left=first.right;
    else view.top=first.bottom;
    double zoom=(double)first.zoom*(view.right-view.left)/(first.right-first.left);
    if(zoom<1 || zoom>65535) return false;
    view.zoom=(unsigned short)(zoom+0.5);
    return true;
}
// Adapt retail 4:3 zoom to the selected display aspect without stretching.
inline bool CoOp_ApplyDisplayAspect(CoOpRetailViewport& view, int width, int height)
{
    if(width<=0 || height<=0) return false;
    // Original camera FOV was tuned for a 4:3 display; preserve its vertical FOV.
    double zoom=(double)view.zoom*width*3.0/(height*4.0);
    if(zoom<1 || zoom>65535) return false;
    view.zoom=(unsigned short)(zoom+0.5);
    return true;
}
#endif
