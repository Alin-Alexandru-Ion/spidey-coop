// Co-op / presentation: Render co-op HUD, effects, display modes, and capture diagnostics.
#pragma once
#ifndef COOP_UI_REMAKE_H
#define COOP_UI_REMAKE_H

// Optional 8x replacements for the five bits.psx HUD textures. The original
// frame dimensions and UV metadata stay intact; only the DirectDraw surface
// changes. Missing or unsupported replacements leave the retail art in place.
static const u32 kCoOpUiRemakeOffsets[] = {
    0x00005468, 0x00005C7C, 0x00005E90, 0x000062A4, 0x000064B8
};
static IDirectDrawSurface7* gCoOpUiRemakeSurfaces[GLOBAL_TEXTURE_COUNT] = {0};

// Initialize co-op ui remake path with no active retail ownership.
static bool CoOpUiRemakePath(char* out, const char* leaf)
{
    DWORD length = GetModuleFileNameA(0, out, MAX_PATH);
    if (!length || length >= MAX_PATH) return false;
    char* slash = strrchr(out, '\\');
    if (!slash) return false;
    ++slash;
    const char* folder = "ui-remakes\\";
    if ((size_t)(slash - out) + strlen(folder) + strlen(leaf) >= MAX_PATH)
        return false;
    strcpy(slash, folder);
    strcat(slash, leaf);
    return true;
}

// Initialize co-op ui remake log with no active retail ownership.
static void CoOpUiRemakeLog(const char* name, const char* status)
{
    char path[MAX_PATH];
    if (!CoOpUiRemakePath(path, "..\\ui-remakes.log")) return;
    FILE* log = fopen(path, "a");
    if (!log) return;
    fprintf(log, "%s: %s\n", name, status);
    fclose(log);
}

// Initialize co-op ui remake source offset with no active retail ownership.
static u32 CoOpUiRemakeSourceOffset(const void* source)
{
    if (!source) return 0;
    const u32 address = reinterpret_cast<u32>(source);
    for (int region = 0; region < MAXPSX; ++region)
    {
        const SPSXRegion& item = G_PSXREGION[region];
        if (!item.pPSX || _strnicmp(item.Filename, "bits", 4) || item.Filename[4])
            continue;
        const u32 base = reinterpret_cast<u32>(item.pPSX);
        if (address < base) continue;
        const u32 relative = address - base;
        for (unsigned i = 0; i < sizeof(kCoOpUiRemakeOffsets)/sizeof(kCoOpUiRemakeOffsets[0]); ++i)
        {
            const u32 offset = kCoOpUiRemakeOffsets[i];
            // The caller normally passes the record's pixel pointer (+0x14).
            // Accept the nearby record-data pointer variants as well.
            if (relative >= offset + 0x10 && relative <= offset + 0x20)
                return offset;
        }
    }
    return 0;
}

// Initialize co-op ui remake surface in format with no active retail ownership.
static bool CoOpUiRemakeSurfaceInFormat(const u8* pixels, u32 width, u32 height,
    bool fullColor, IDirectDrawSurface7** out)
{
    *out = 0;
    if (!G_MOVIE_DD7)
        return false;
    DDSURFACEDESC2 desc;
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    desc.dwWidth = width;
    desc.dwHeight = height;
    desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
    desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    desc.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
    desc.ddpfPixelFormat.dwRGBBitCount = fullColor ? 32 : 16;
    desc.ddpfPixelFormat.dwRBitMask = fullColor ? 0x00FF0000 : 0x0F00;
    desc.ddpfPixelFormat.dwGBitMask = fullColor ? 0x0000FF00 : 0x00F0;
    desc.ddpfPixelFormat.dwBBitMask = fullColor ? 0x000000FF : 0x000F;
    desc.ddpfPixelFormat.dwRGBAlphaBitMask = fullColor ? 0xFF000000 : 0xF000;

    IDirectDrawSurface7* upload = 0;
    if (FAILED(G_MOVIE_DD7->CreateSurface(&desc, &upload, 0))) return false;
    if (FAILED(upload->Lock(0, &desc, DDLOCK_WRITEONLY, 0)))
    {
        upload->Release();
        return false;
    }
    for (u32 y = 0; y < height; ++y)
    {
        u8* dest = static_cast<u8*>(desc.lpSurface) + y * desc.lPitch;
        const u8* source = pixels + y * width * 4;
        if (fullColor) memcpy(dest, source, width * 4);
        else for (u32 x = 0; x < width; ++x)
        {
            const u8* pixel = source + x * 4; // BGRA
            const u16 packed = (u16)(((pixel[3] >> 4) << 12) |
                ((pixel[2] >> 4) << 8) | ((pixel[1] >> 4) << 4) |
                (pixel[0] >> 4));
            reinterpret_cast<u16*>(dest)[x] = packed;
        }
    }
    upload->Unlock(0);

    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    desc.dwWidth = width;
    desc.dwHeight = height;
    desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    desc.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
    desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    desc.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
    desc.ddpfPixelFormat.dwRGBBitCount = fullColor ? 32 : 16;
    desc.ddpfPixelFormat.dwRBitMask = fullColor ? 0x00FF0000 : 0x0F00;
    desc.ddpfPixelFormat.dwGBitMask = fullColor ? 0x0000FF00 : 0x00F0;
    desc.ddpfPixelFormat.dwBBitMask = fullColor ? 0x000000FF : 0x000F;
    desc.ddpfPixelFormat.dwRGBAlphaBitMask = fullColor ? 0xFF000000 : 0xF000;
    IDirectDrawSurface7* replacement = 0;
    const bool ok = SUCCEEDED(G_MOVIE_DD7->CreateSurface(&desc, &replacement, 0)) &&
        SUCCEEDED(replacement->Blt(0, upload, 0, DDBLT_WAIT, 0));
    upload->Release();
    if (!ok)
    {
        if (replacement) replacement->Release();
        return false;
    }
    *out = replacement;
    return true;
}

// Initialize co-op ui remake surface with no active retail ownership.
static bool CoOpUiRemakeSurface(const u8* pixels, u32 width, u32 height,
    IDirectDrawSurface7** out, bool* fullColor)
{
    if (CoOpUiRemakeSurfaceInFormat(pixels, width, height, true, out))
    {
        *fullColor = true;
        return true;
    }
    *fullColor = false;
    return CoOpUiRemakeSurfaceInFormat(pixels, width, height, false, out);
}

// Initialize co-op ui remake try apply with no active retail ownership.
static void CoOpUiRemakeTryApply(i32 textureId, const void* source, i32 logicalWidth,
    i32 logicalHeight)
{
    const u32 offset = CoOpUiRemakeSourceOffset(source);
    if (!offset || textureId < 8 || textureId >= GLOBAL_TEXTURE_COUNT) return;
    SPCTexture& texture = G_GLOBAL_TEXTURES[textureId];
    if (!texture.mD3DTex || texture.mSplit || G_LOWGRAPHICS) return;

    char name[64];
    sprintf(name, "bits_%08X.bgra", offset);
    char path[MAX_PATH];
    if (!CoOpUiRemakePath(path, name)) return;
    FILE* file = fopen(path, "rb");
    if (!file) { CoOpUiRemakeLog(name, "missing; retail texture kept"); return; }
    u32 header[3];
    const bool headerOk = fread(header, sizeof(u32), 3, file) == 3 &&
        header[0] == 0x38524955 && logicalWidth > 0 && logicalHeight > 0 &&
        header[1] == (u32)logicalWidth * 8 &&
        header[2] == (u32)logicalHeight * 8 &&
        header[1] <= 1024 && header[2] <= 1024;
    if (!headerOk)
    {
        fclose(file);
        CoOpUiRemakeLog(name, "invalid dimensions or header; retail texture kept");
        return;
    }
    const size_t size = (size_t)header[1] * header[2] * 4;
    u8* pixels = static_cast<u8*>(malloc(size));
    if (!pixels) { fclose(file); CoOpUiRemakeLog(name, "out of memory"); return; }
    const bool readOk = fread(pixels, 1, size, file) == size && fgetc(file) == EOF;
    fclose(file);
    IDirectDrawSurface7* replacement = 0;
    bool fullColor = false;
    const bool surfaceOk = readOk &&
        CoOpUiRemakeSurface(pixels, header[1], header[2], &replacement, &fullColor);
    free(pixels);
    if (!surfaceOk)
    {
        CoOpUiRemakeLog(name, readOk ? "high-resolution surface unsupported; retail texture kept" :
            "truncated file; retail texture kept");
        return;
    }
    texture.mD3DTex->Release();
    texture.mD3DTex = replacement;
    gCoOpUiRemakeSurfaces[textureId] = replacement;
    texture.mAlpha |= 1;
    CoOpUiRemakeLog(name, fullColor ? "8x surface installed (32-bit)" :
        "8x surface installed (16-bit fallback)");
}

#endif
