// Co-op / presentation: Render co-op HUD, effects, display modes, and capture diagnostics.
#pragma once
#ifndef COOP_DISPLAY_MSAA_RUNTIME_H
#define COOP_DISPLAY_MSAA_RUNTIME_H

// DDrawCompat reads process-specific options when the game starts. Keep its
// setting alongside the executable so the display menu can persist it.
static CMenu* gCoOpDisplayMsaaMenu;
static char gCoOpDisplayMsaaLabel[80];
static bool gCoOpDisplayMsaaSaveFailed;
static int gCoOpDisplayMsaaValue;

// Build a path beside the executable for the DDrawCompat setting file.
static bool CoOp_DisplayMsaaPath(char* out, const char* leaf)
{
    DWORD length=GetModuleFileNameA(0,out,MAX_PATH);
    if(!length || length>=MAX_PATH) return false;
    char* slash=strrchr(out,'\\');
    if(!slash || (size_t)(slash-out)+strlen(leaf)>=MAX_PATH) return false;
    strcpy(slash+1,leaf);
    return true;
}

// Parse the anti-aliasing value from one DDrawCompat config line.
static const char* CoOp_DisplayMsaaValue(const char* line)
{
    while(*line==' ' || *line=='\t') ++line;
    if(*line=='#' || *line==';') return 0;
    const char* key="Antialiasing";
    if(_strnicmp(line,key,strlen(key))) return 0;
    line+=strlen(key);
    while(*line==' ' || *line=='\t') ++line;
    if(*line!='=') return 0;
    ++line;
    while(*line==' ' || *line=='\t') ++line;
    return line;
}

// Read the current anti-aliasing preference for the display menu.
static int CoOp_DisplayMsaaReadFile(const char* path)
{
    FILE* file=fopen(path,"rb");
    if(!file) return -1;
    int result=-1;
    char line[1024];
    while(fgets(line,sizeof(line),file)) {
        const char* value=CoOp_DisplayMsaaValue(line);
        if(!value) continue;
        if(!_strnicmp(value,"msaa8x",6)) result=8;
        else if(!_strnicmp(value,"msaa4x",6)) result=4;
        else if(!_strnicmp(value,"msaa2x",6)) result=2;
        else result=0;
    }
    fclose(file);
    return result;
}

// Format the anti-aliasing option shown by the native menu.
static void CoOp_DisplayMsaaLabel()
{
    if(gCoOpDisplayMsaaSaveFailed)
        strcpy(gCoOpDisplayMsaaLabel,"ANTI-ALIASING: SAVE FAILED");
    else if(gCoOpDisplayMsaaValue)
        sprintf(gCoOpDisplayMsaaLabel,"ANTI-ALIASING: %dX (RESTART)",gCoOpDisplayMsaaValue);
    else strcpy(gCoOpDisplayMsaaLabel,"ANTI-ALIASING: OFF (RESTART)");
    if(gCoOpDisplayMsaaMenu && gCoOpDisplayMsaaMenu->mNumLines>3)
        gCoOpDisplayMsaaMenu->mEntry[3].name=gCoOpDisplayMsaaLabel;
}

// Persist the anti-aliasing choice for the next game launch.
static bool CoOp_DisplayMsaaSave(int value)
{
    char path[MAX_PATH],temp[MAX_PATH];
    if(!CoOp_DisplayMsaaPath(path,"DDrawCompat-SpideyPC.ini") ||
        !CoOp_DisplayMsaaPath(temp,"DDrawCompat-SpideyPC.ini.tmp")) return false;
    FILE* input=fopen(path,"rb");
    FILE* output=fopen(temp,"wb");
    if(!output) {if(input)fclose(input);return false;}
    bool found=false,ok=true;
    char line[1024];
    if(input) {
        while(fgets(line,sizeof(line),input)) {
            if(CoOp_DisplayMsaaValue(line)) {
                if(!found && fprintf(output,"Antialiasing = %s\r\n",value?"msaa4x":"off")<0)
                    ok=false;
                found=true;
            } else if(fputs(line,output)==EOF) ok=false;
        }
        if(ferror(input)) ok=false;
        fclose(input);
    }
    if(!found && fprintf(output,"Antialiasing = %s\r\n",value?"msaa4x":"off")<0)
        ok=false;
    if(fclose(output)!=0) ok=false;
    if(ok) ok=MoveFileExA(temp,path,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
    if(!ok) DeleteFileA(temp);
    return ok;
}

static void __fastcall CoOp_DisplayMsaaAdd(CMenu* menu,void*,const char* text)
{
    typedef void (__fastcall *AddFn)(CMenu*,void*,const char*);
    AddFn add=reinterpret_cast<AddFn>(0x0043FFF0);
    add(menu,0,text);
    // Reserve enough menu width for either value and the restart notice.
    add(menu,0,"ANTI-ALIASING: 4X (RESTART)");
    gCoOpDisplayMsaaMenu=menu;
    gCoOpDisplayMsaaSaveFailed=false;
    char path[MAX_PATH];
    gCoOpDisplayMsaaValue=0;
    if(CoOp_DisplayMsaaPath(path,"DDrawCompat.ini")) {
        int value=CoOp_DisplayMsaaReadFile(path);
        if(value>=0) gCoOpDisplayMsaaValue=value;
    }
    if(CoOp_DisplayMsaaPath(path,"DDrawCompat-SpideyPC.ini")) {
        int value=CoOp_DisplayMsaaReadFile(path);
        if(value>=0) gCoOpDisplayMsaaValue=value;
    }
    CoOp_DisplayMsaaLabel();
}

// Prepare display msaa trigger for the owning viewport.
static u8 __cdecl CoOp_DisplayMsaaTrigger(u32 mask,i32 a,i32 b)
{
    u8 pressed=reinterpret_cast<u8 (__cdecl*)(u32,i32,i32)>(0x0050C180)(mask,a,b);
    if(!pressed || !gCoOpDisplayMsaaMenu || gCoOpDisplayMsaaMenu->mLine!=3)
        return pressed;
    int next=gCoOpDisplayMsaaValue?0:4;
    gCoOpDisplayMsaaSaveFailed=!CoOp_DisplayMsaaSave(next);
    if(!gCoOpDisplayMsaaSaveFailed) gCoOpDisplayMsaaValue=next;
    CoOp_DisplayMsaaLabel();
    return 0; // Do not invoke the retail resolution-apply action for this row.
}

// Install install display msaa hooks at the supported retail call site, preserving its fallback.
static bool CoOp_InstallDisplayMsaaHooks()
{
    const u32 sites[]={0x0050DA72,0x0050DCBD};
    const u32 targets[]={0x0043FFF0,0x0050C180};
    const u32 hooks[]={reinterpret_cast<u32>(&CoOp_DisplayMsaaAdd),
        reinterpret_cast<u32>(&CoOp_DisplayMsaaTrigger)};
    if(sizeof(CMenu)!=0x53C) return false;
    for(unsigned i=0;i<2;++i) {
        const u8* code=reinterpret_cast<const u8*>(sites[i]);
        if(code[0]!=0xE8 || sites[i]+5+*reinterpret_cast<const i32*>(code+1)!=targets[i])
            return false;
    }
    for(unsigned j=0;j<2;++j) {
        DWORD old,unused;
        if(!VirtualProtect(reinterpret_cast<void*>(sites[j]),5,PAGE_EXECUTE_READWRITE,&old))
            return false;
        *reinterpret_cast<i32*>(sites[j]+1)=hooks[j]-sites[j]-5;
        VirtualProtect(reinterpret_cast<void*>(sites[j]),5,old,&unused);
        FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(sites[j]),5);
    }
    return true;
}

#endif
