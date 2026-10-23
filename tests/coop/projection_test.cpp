#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "coop/camera/coop_view.h"
#include "coop/presentation/coop_effects.h"
static unsigned short viewport[10]; static void* entryPointer;
__declspec(naked) void RunProjection()
{
    __asm {
        pushad
        sub esp, 0D0h
        mov esi, offset viewport
        xor ebx,ebx
        mov eax,entryPointer
        jmp eax
    }
}
__declspec(naked) void FinishProjection()
{
    __asm {
        add esp,0D0h
        popad
        ret
    }
}
// Exercise the supported EXE's actual fixed-point sprite projection too.
// No entrypoint/imports: this routine only touches its relocated math globals.
static void TestEffects(unsigned char* image) {
    for(unsigned i=0x6DBC0;i+4<0x6DCE0;++i) {
        DWORD value=*(DWORD*)(image+i);
        if((value>=0x610B20 && value<=0x610C20) || (value>=0x54F03C && value<=0x54F044))
            *(DWORD*)(image+i)=value-0x400000+(DWORD)image;
    }
    *(long*)(image+0x14F03C)=276;*(long*)(image+0x14F040)=512;*(long*)(image+0x14F044)=240;
    float* m=(float*)(image+0x16E570);
    int width=*(int*)(image+0x168154),height=*(int*)(image+0x168158);
    CoOpEffectProjection projection;assert(projection.Set(m,width,height,512,240));
    for(int orientation=0;orientation<3;++orientation) {
        short* rotation=(short*)(image+0x210B20);
        memset(rotation,0,0x100);
        long* v=(long*)(image+0x210BB0);
        if(!orientation) {rotation[0]=6553;rotation[4]=rotation[8]=4096;v[0]=70;v[1]=40;v[2]=500;}
        else if(orientation==1) {rotation[2]=6553;rotation[4]=4096;rotation[6]=-4096;v[0]=-500;v[1]=40;v[2]=70;}
        else {rotation[0]=6553;rotation[5]=4096;rotation[7]=-4096;v[0]=70;v[1]=-500;v[2]=40;}
        ((void (__cdecl*)())(image+0x6DBC0))();
        long x=*(long*)(image+0x210BA0),y=*(long*)(image+0x210BA4),z=*(long*)(image+0x210BA8);
        assert(z==500);
        projection.Remap(x,y,z,276,512,240,6553,4096);
        double expectedX=(m[8]+m[0]*70/500)*512/width;
        double expectedY=(m[9]+m[5]*40/500)*240/height;
        assert(fabs(x-expectedX)<2.0 && fabs(y-expectedY)<2.0);
    }
}
int main(int argc,char** argv)
{
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    assert(argc==2);
    FILE* f=fopen(argv[1],"rb"); assert(f);
    fseek(f,0,SEEK_END); long length=ftell(f); rewind(f);
    unsigned char* bytes=new unsigned char[length];
    assert(fread(bytes,1,length,f)==(unsigned long)length); fclose(f);
    IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)bytes;
    IMAGE_NT_HEADERS* nt=(IMAGE_NT_HEADERS*)(bytes+dos->e_lfanew);
    unsigned char* image=(unsigned char*)VirtualAlloc(0,
        nt->OptionalHeader.SizeOfImage,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE);
    assert(image);
    memcpy(image,bytes,nt->OptionalHeader.SizeOfHeaders);
    IMAGE_SECTION_HEADER* s=IMAGE_FIRST_SECTION(nt);
    for(int i=0;i<nt->FileHeader.NumberOfSections;i++)
        memcpy(image+s[i].VirtualAddress,bytes+s[i].PointerToRawData,s[i].SizeOfRawData);
    DWORD delta=(DWORD)image-nt->OptionalHeader.ImageBase;
    IMAGE_DATA_DIRECTORY rd=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    DWORD consumed=0;
    while(consumed<rd.Size) {
        IMAGE_BASE_RELOCATION* block=(IMAGE_BASE_RELOCATION*)(image+rd.VirtualAddress+consumed);
        if(block->SizeOfBlock<8) break;
        WORD* fixes=(WORD*)(block+1);
        for(DWORD k=0;k<(block->SizeOfBlock-8)/2;k++)
            if((fixes[k]>>12)==IMAGE_REL_BASED_HIGHLOW)
                *(DWORD*)(image+block->VirtualAddress+(fixes[k]&4095))+=delta;
        consumed+=block->SizeOfBlock;
    }
    // Execute only projection arithmetic, never an EXE entrypoint or imports.
    *(DWORD*)(image+0x733e1)=(DWORD)image+0x168154;
    *(DWORD*)(image+0x733e7)=(DWORD)image+0x21b5fc;
    *(DWORD*)(image+0x73450)=(DWORD)image+0x168158;
    *(DWORD*)(image+0x73456)=(DWORD)image+0x228614;
    *(DWORD*)(image+0x73506)=(DWORD)image+0x150064;
    *(DWORD*)(image+0x7350c)=(DWORD)image+0x13b36c;
    *(DWORD*)(image+0x73566)=(DWORD)image+0x13b390;
    *(DWORD*)(image+0x73574)=(DWORD)image+0x13b390;
    *(DWORD*)(image+0x735f2)=(DWORD)image+0x13b27c;
    *(DWORD*)(image+0x7368c)=(DWORD)image+0x16e570;
    *(DWORD*)(image+0x73693)=(DWORD)image+0x16e57c;
    *(DWORD*)(image+0x736a8)=(DWORD)image+0x16e570;
    entryPointer=image+0x733DF;
    image[0x736C2]=0xE9;
    *(long*)(image+0x736C3)=(long)FinishProjection-(long)(image+0x736C7);
    FlushInstructionCache(GetCurrentProcess(),image,nt->OptionalHeader.SizeOfImage);
    *(long*)(image+0x168154)=640; *(long*)(image+0x168158)=480;
    *(long*)(image+0x21B5FC)=512; *(long*)(image+0x228614)=240;

    CoOpRetailViewport full = {512,240,0,0,10,6000,2365,0,0,0}, view;
    assert(sizeof(full)==20);
    assert(!CoOp_BuildPrimaryView(full,COOP_SPLIT_OFF,view));
    float baseline[16];
    memcpy(viewport,&full,sizeof(full)); RunProjection();
    memcpy(baseline,image+0x16E570,sizeof(baseline));
    for(int layout=COOP_SPLIT_LEFT_RIGHT;layout<=COOP_SPLIT_TOP_BOTTOM;layout++)
    {
        assert(CoOp_BuildPrimaryView(full,(CoOpSplitLayout)layout,view));
        memcpy(viewport,&view,sizeof(view)); RunProjection();TestEffects(image);
        float* m=(float*)(image+0x16E570);
        float heightRatio=(float)(view.bottom-view.top)/(full.bottom-full.top);
        if(layout==COOP_SPLIT_TOP_BOTTOM) heightRatio*=1.35f;
        assert(fabs(m[0]/baseline[0]-heightRatio)<0.001);
        assert(fabs(m[5]/baseline[5]-heightRatio)<0.001);
        assert(fabs(m[0]/m[5]-baseline[0]/baseline[5])<0.001);
        assert(fabs(m[8]-(layout==COOP_SPLIT_LEFT_RIGHT?160.0:320.0))<0.001);
        assert(fabs(m[9]-(layout==COOP_SPLIT_TOP_BOTTOM?120.0:240.0))<0.001);
        assert(m[10]==baseline[10] && m[14]==baseline[14]);
        printf("PASS retail projection layout %d: focal X=%.3f Y=%.3f, center %.1f,%.1f\n",
            layout,m[0],m[5],m[8],m[9]);
    }

    full.zoom=2365;
    assert(CoOp_ApplyDisplayAspect(full,1280,720));
    *(long*)(image+0x168154)=1280; *(long*)(image+0x168158)=720;
    memcpy(viewport,&full,sizeof(full)); RunProjection();
    float wideFocal=((float*)(image+0x16E570))[0];
    assert(fabs(wideFocal/baseline[0]-1.5)<0.002);
    for(int layout2=COOP_SPLIT_LEFT_RIGHT;layout2<=COOP_SPLIT_TOP_BOTTOM;layout2++)
    {
        assert(CoOp_BuildSecondaryView(full,(CoOpSplitLayout)layout2,view));
        memcpy(viewport,&view,sizeof(view)); RunProjection();TestEffects(image);
        float* wide=(float*)(image+0x16E570);
        assert(fabs(wide[0]-wide[5])<0.001);
        assert(fabs(wide[8]-(layout2==COOP_SPLIT_LEFT_RIGHT?960.0:640.0))<0.001);
        assert(fabs(wide[9]-(layout2==COOP_SPLIT_TOP_BOTTOM?540.0:360.0))<0.001);
    }
    puts("PASS 1280x720 aspect correction and both P2 optical centers");
    assert(CoOp_BuildPrimaryView(full,COOP_VIEW_PLAYER_TWO,view));
    assert(!memcmp(&full,&view,sizeof(full)));
    assert(CoOp_BuildSecondaryView(full,COOP_VIEW_PLAYER_TWO,view));
    assert(!memcmp(&full,&view,sizeof(full)));
    memcpy(viewport,&view,sizeof(view));RunProjection();TestEffects(image);
    float* solo=(float*)(image+0x16E570);
    assert(fabs(solo[0]-wideFocal)<.001 && fabs(solo[0]-solo[5])<.001);
    assert(fabs(solo[8]-640)<.001 && fabs(solo[9]-360)<.001);
    CoOpSplitLayout selected=COOP_SPLIT_LEFT_RIGHT;
    for(int cycle=0;cycle<12;++cycle) {
        assert(CoOp_ViewShowsOwner(selected,0));
        assert(CoOp_ViewShowsOwner(selected,1)==(selected!=COOP_VIEW_PLAYER_TWO));
        assert(CoOp_ViewShowsOwner(selected,2)==(selected!=COOP_SPLIT_OFF));
        selected=CoOp_NextView(selected);
    }
    assert(selected==COOP_SPLIT_LEFT_RIGHT);
    puts("PASS P2 full-screen retail projection and repeated four-view/HUD cycles");
    full.right=0;
    assert(!CoOp_BuildPrimaryView(full,COOP_SPLIT_LEFT_RIGHT,view));
    full.right=512; full.zoom=0;
    assert(!CoOp_BuildPrimaryView(full,COOP_SPLIT_LEFT_RIGHT,view));
    full.zoom=65535;
    assert(!CoOp_BuildPrimaryView(full,COOP_SPLIT_TOP_BOTTOM,view));
    full.zoom=2365; full.left=10; full.top=8; full.right=511; full.bottom=239;
    assert(CoOp_BuildPrimaryView(full,COOP_SPLIT_LEFT_RIGHT,view));
    assert(view.left==10 && view.right==260 && view.top==8 && view.bottom==239);
    puts("PASS viewport validation, offset/odd rectangles, zoom bounds");
    return 0;
}
