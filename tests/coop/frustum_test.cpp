#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "coop/camera/coop_view.h"
#define COOP_RETAIL_ADDRESS(x) ((x)+0x20000000)
#include "retail_frustum_helpers.h"
static CoOpRetailViewport pane;
__declspec(naked) void RunPlanes()
{
    __asm {
        pushad
        sub esp, 100h
        mov esi, offset pane
        xor ebx,ebx
        xor edi,edi
        mov eax,20472F32h
        jmp eax
    }
}
__declspec(naked) void FinishPlanes()
{
    __asm {
        add esp,110h
        popad
        ret
    }
}
int __cdecl Root(int value) { return (int)sqrt((double)value); }
void Jump(unsigned long address,void* target)
{
    *(unsigned char*)address=0xE9;
    *(long*)(address+1)=(long)target-(address+5);
}
int main(int argc,char** argv)
{
    SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    assert(argc==2);
    FILE* f=fopen(argv[1],"rb"); assert(f);
    fseek(f,0,SEEK_END); long length=ftell(f); rewind(f);
    unsigned char* bytes=new unsigned char[length];
    assert(fread(bytes,1,length,f)==(unsigned long)length); fclose(f);
    IMAGE_NT_HEADERS* nt=(IMAGE_NT_HEADERS*)(bytes+((IMAGE_DOS_HEADER*)bytes)->e_lfanew);
    // Test executable is linked elsewhere. Relocate only isolated arithmetic.
    unsigned char* image=(unsigned char*)VirtualAlloc((void*)0x20400000,
        nt->OptionalHeader.SizeOfImage,MEM_RESERVE|MEM_COMMIT,PAGE_EXECUTE_READWRITE);
    assert(image==(void*)0x20400000);
    IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(nt);
    for(int i=0;i<nt->FileHeader.NumberOfSections;++i)
        memcpy(image+section[i].VirtualAddress,bytes+section[i].PointerToRawData,
            section[i].SizeOfRawData);
    // Absolute operands in the isolated frustum and GTE arithmetic blocks.
    const unsigned long fixups[]={0x46d702,0x46d707,0x46d70e,0x46d717,0x46d725,0x46d735,0x46d741,0x46d74e,0x46d754,0x46d75a,0x46d768,0x46d76e,0x46d773,0x46d779,0x46d77f,0x46d796,0x46d79e,0x46d7a7,0x46d7b7,0x46d7c4,0x46d7d6,0x46d7e7,0x46d7f4,0x46d806,0x46d817,0x46d824,0x46d836,0x46d848,0x46d851,0x46d85a,0x46d863,0x46d878,0x46d881,0x46d88a,0x46d893,0x46d8a9,0x46d8b3,0x46d8bc,0x46d8d9,0x46d8e3,0x46d8ec,0x46d909,0x46d913,0x46d91c,0x46d93a,0x46d945,0x46d94f,0x46d96a,0x46d975,0x46d97f,0x46d995,0x46d9a5,0x46d9b6,0x46d9be,0x46d9c7,0x46d9d3,0x46d9dd,0x46da17,0x46da21,0x46da2c,0x46da43,0x46da4a,0x46da51,0x46da5b,0x46da62,0x46da69,0x46da7b,0x46da87,0x46da93,0x46da9f,0x46daa9,0x46dab6,0x46dac3,0x46dad4,0x46dada,0x46dadf,0x46daf3,0x46dafa,0x46db01,0x46db0b,0x46db12,0x46db19,0x46db2b,0x46db37,0x46db43,0x46db4f,0x46db59,0x46db66,0x46db75,0x46db7b,0x46db86,0x46db8d,0x46db93,0x46dba2,0x46dba8,0x46dbb7,0x46dbc3,0x46dbca,0x46dbd1,0x46dbdb,0x46dbe2,0x46dbe9,0x46dbfb,0x46dc07,0x46dc13,0x46dc1f,0x46dc29,0x46dc34,0x46dc3b,0x46dc4f,0x46dc55,0x46dc60,0x46dc67,0x46dc71,0x46dc7f,0x46dc84,0x46dc8f,0x46dc9e,0x46dcb4,0x46dcbd,0x46dcca,0x46dce1,0x46dcea,0x46dd01,0x46dd0b,0x46dd11,0x46dd1b,0x46dd21,0x46dd2c,0x46dd43,0x46dd4a,0x46dd51,0x46dd5b,0x46dd62,0x46dd69,0x46dd7b,0x46dd87,0x46dd93,0x46dd9f,0x46dda9,0x46ddb6,0x46ddc3,0x46ddd4,0x46ddda,0x46dddf,0x46ddf3,0x46ddfa,0x46de01,0x46de0b,0x46de12,0x46de19,0x46de2b,0x46de37,0x46de43,0x46de4f,0x46de59,0x46de66,0x46de7e,0x46de84,0x46de8a,0x46de90,0x46de96,0x46de9b,0x46dea1,0x46dea7,0x46deb3,0x46deba,0x46dec1,0x46decb,0x46ded2,0x46ded9,0x46deeb,0x46def7,0x46df03,0x46df0f,0x46df19,0x46df26,0x46df33,0x46df44,0x46df4a,0x46df4f,0x46df66,0x46df76,0x46df81,0x46df87,0x46dfa1,0x46dfa7,0x46dfbd,0x46dfc3,0x46dfd9,0x46dfdf,0x46dff5,0x46dffe,0x472f62,0x472f81,0x472f88,0x472f8f,0x472f9c,0x472fa3,0x472faa,0x472fb1,0x472fc0,0x472fcb,0x473009,0x473014,0x47301b,0x473029,0x473033,0x473039,0x47303f,0x473048,0x47305b,0x473093,0x47309e,0x4730a5,0x4730ac,0x4730b4,0x4730be,0x4730c9,0x4730cf,0x4730d5,0x473109,0x473114,0x47311b,0x473122,0x473133,0x47313f,0x473145,0x47314b,0x473180,0x47318b,0x473192,0x473199,0x47319f,0x4731b3,0x4731bc,0x4731c2,0x4731c8};
    for(int fix=0;fix<sizeof(fixups)/sizeof(fixups[0]);++fix)
        *(unsigned long*)(image+fixups[fix]-0x400000)+=0x20000000;
    // Stop before render setup reaches any renderer. Substitute sqrt conversion
    // to avoid the retail CRT's uninitialized import table.
    Jump(0x204731cc,FinishPlanes); Jump(0x2046d430,Root);
    FlushInstructionCache(GetCurrentProcess(),image,nt->OptionalHeader.SizeOfImage);
    *(long*)0x20654f58=4096; *(long*)0x20654f5c=4096;
    CoOpRetailViewport full={512,240,0,0,10,6000,2365,0,0,0};
    short rotation[16]={4096,0,0,0,4096,0,0,0,4096};
    unsigned char shape[0x50], expectedA[18],expectedB[18];
    for(int layout=1;layout<=2;++layout)
    {
        assert(CoOp_BuildPrimaryView(full,(CoOpSplitLayout)layout,pane));
        RunPlanes(); memcpy(shape,(void*)0x2065ceb8,sizeof(shape));
        CoOp_UsePaneFrustum(shape,rotation);
        memcpy(expectedA,(void*)0x20628648,18);
        memcpy(expectedB,(void*)0x20628620,18);
        assert(CoOp_BuildSecondaryView(full,(CoOpSplitLayout)layout,pane));
        RunPlanes();
        assert(memcmp(shape,(void*)0x2065ceb8,sizeof(shape))==0);
        CoOp_UsePaneFrustum(shape,rotation);
        assert(memcmp(expectedA,(void*)0x20628648,18)==0);
        assert(memcmp(expectedB,(void*)0x20628620,18)==0);
        printf("PASS layout %d: retail offset frustum matches pane shape\n",layout);
    }
    // Different P2 orientation must rotate both sets, without changing shape.
    short yaw[16]={0,0,4096,0,4096,0,-4096,0,0};
    CoOp_UsePaneFrustum(shape,yaw);
    assert(memcmp(expectedA,(void*)0x20628648,18)!=0);
    assert(memcmp(expectedB,(void*)0x20628620,18)!=0);
    assert(memcmp(shape,(void*)0x2065ceb8,sizeof(shape))==0);
    puts("PASS independent P2 camera rotation");
    return 0;
}
