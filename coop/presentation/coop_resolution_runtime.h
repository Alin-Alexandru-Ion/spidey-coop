// Co-op / presentation: Render co-op HUD, effects, display modes, and capture diagnostics.
#pragma once
#ifndef COOP_RESOLUTION_RUNTIME_H
#define COOP_RESOLUTION_RUNTIME_H

// The retail DirectDraw callback at 0x004FD770 admits only the five dimensions
// in its fixed table. Preserve that callback for every existing mode and add
// the requested widescreen modes when DirectDraw actually enumerates them.
static bool CoOp_IsAddedResolution(DWORD width, DWORD height)
{
    return (width == 1920 && height == 1080) ||
        (width == 2560 && height == 1440) ||
        (width == 3840 && height == 2160);
}

static HRESULT WINAPI CoOp_EnumerateDisplayMode(LPDDSURFACEDESC2 desc, void* context)
{
    typedef HRESULT (WINAPI *RetailEnumFn)(LPDDSURFACEDESC2, void*);
    RetailEnumFn retail = reinterpret_cast<RetailEnumFn>(0x004FD770);
    if (!desc || !context || !CoOp_IsAddedResolution(desc->dwWidth, desc->dwHeight))
        return retail(desc, context);

    DXVideoModeContext* modes = reinterpret_cast<DXVideoModeContext*>(context);
    if (modes->mNumEntries >= 64) return FALSE;
    if (desc->ddpfPixelFormat.dwRGBBitCount < 16) return TRUE;
    for (int i = 0; i < modes->mNumEntries; ++i)
    {
        const DDSURFACEDESC2& existing = modes->mSurfaces[i];
        if (existing.dwWidth == desc->dwWidth && existing.dwHeight == desc->dwHeight &&
            existing.ddpfPixelFormat.dwRGBBitCount == desc->ddpfPixelFormat.dwRGBBitCount)
            return TRUE;
    }
    const int index = modes->mNumEntries++;
    modes->mSurfaces[index] = *desc;
    modes->mFlags[index] = 5; // retail hardware mode (4) plus Z-buffer/depth support (1)
    return TRUE;
}

// Install install resolution hook at the supported retail call site, preserving its fallback.
static bool CoOp_InstallResolutionHook()
{
    // PUSH 0x004FD770 is the callback argument to retail EnumDisplayModes.
    if (*reinterpret_cast<const u8*>(0x004FF281) != 0x68 ||
        *reinterpret_cast<const u32*>(0x004FF282) != 0x004FD770)
        return false;
    *reinterpret_cast<u32*>(0x004FF282) =
        reinterpret_cast<u32>(&CoOp_EnumerateDisplayMode);
    return true;
}

#endif
