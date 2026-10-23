#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "coop/world/coop_world_trace.h"
struct Vec {long vx,vy,vz;};
struct Item {unsigned short mFlags; Vec mPos; Item* mNextItem;};
int main() {
    CoOpWorldTrace<Item,8> trace;
    Item a={0,{0,0,0},0},b={0,{4096,8192,12288},0};
    a.mNextItem=&b;
    trace.Capture(&a,0,7,8);
    assert(trace.Events()==0);
    b.mFlags=0x8000;trace.Capture(&a,4,7,8);
    assert(trace.Events()==0); // camera culling is not a lifetime event
    b.mFlags|=1;trace.Capture(&a,5,7,8);
    assert(trace.Events()==1);
    a.mNextItem=0;trace.Capture(&a,0,7,8);
    assert(trace.Events()==2); // no read through the removed pointer
    b.mFlags=0x20;a.mNextItem=&b;trace.Capture(&a,1,7,8);
    assert(trace.Events()==3);
    assert(a.mFlags==0 && b.mFlags==0x20); // observer changes nothing
    FILE* f=fopen("world-trace-output.txt","w+");assert(f);trace.Write(f);rewind(f);
    char text[4096]={0};fread(text,1,sizeof(text)-1,f);fclose(f);
    assert(strstr(text,"kind=2") && strstr(text,"kind=3") && strstr(text,"kind=1"));
    assert(strstr(text,"resources=00000007 grid=00000008"));
    assert(trace.Events()==0);
    for(int i=0;i<140;++i) {b.mFlags^=1;trace.Capture(&a,2,7,8);}
    assert(trace.Events()==128 && trace.Dropped()==12);
    trace.Clear();assert(trace.Events()==0);
    a.mNextItem=&a;trace.Capture(&a,0,7,8); // bounded cycle
    f=fopen("world-trace-output.txt","w+");assert(f);trace.Write(f);rewind(f);
    memset(text,0,sizeof(text));fread(text,1,sizeof(text)-1,f);fclose(f);
    assert(strstr(text,"incomplete=1"));
    puts("PASS passive world trace: flags vs culling, removal, return, bounded event/cycle handling");
    return 0;
}
