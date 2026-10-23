// Co-op / core: Own the two-player roster and scoped compatibility state for retail globals.
#pragma once
#ifndef COOP_CONTEXT_H
#define COOP_CONTEXT_H
#include <string.h>
#include <new>

// Explicit legacy singleton storage only. Never copy player allocations or the
// shared world's actor lists. Each byte is exchanged once, even for aliased tables.
class CoOpContextBank
{
public:
    // Initialize co-op context bank with no active retail ownership.
    CoOpContextBank() : count(0),active(false),ready(false) {}
    // Release the scoped state owned by this object.
    ~CoOpContextBank() {Clear();}
    // Register a retail-global byte range before the context is captured.
    bool Add(void* address,unsigned long bytes) {
        if(ready || !address || !bytes || count==512) return false;
        blocks[count].address=(unsigned char*)address;
        blocks[count].size=bytes;blocks[count].saved=0;++count;return true;
    }
    // Merge overlapping ranges and save one independent copy of their bytes.
    bool Capture() {
        if(ready) return false;
        unsigned long i,j;
        for(i=1;i<count;++i) {
            Block b=blocks[i];j=i;
            while(j && blocks[j-1].address>b.address) {blocks[j]=blocks[j-1];--j;}
            blocks[j]=b;
        }
        for(i=0;i+1<count;) {
            Block& a=blocks[i];Block& b=blocks[i+1];
            if(b.address<=a.address+a.size) {
                if(b.address+b.size>a.address+a.size)
                    a.size=(unsigned long)(b.address+b.size-a.address);
                for(j=i+1;j+1<count;++j) blocks[j]=blocks[j+1];
                --count;
            } else ++i;
        }
        for(i=0;i<count;++i) {
            blocks[i].saved=new(std::nothrow) unsigned char[blocks[i].size];
            if(!blocks[i].saved) {Clear();return false;}
            memcpy(blocks[i].saved,blocks[i].address,blocks[i].size);
        }
        ready=true;return true;
    }
    // Swap this player's saved bytes into the retail singleton addresses.
    bool Enter() {if(!ready || active) return false;Swap();active=true;return true;}
    // Restore the previous player's globals when this scope exits.
    void Leave() {if(active) {Swap();active=false;}}
    // Release owned resources and reset the active roster and scoped retail globals to its inactive state.
    void Clear() {
        Leave();
        for(unsigned long i=0;i<count;++i) delete[] blocks[i].saved;
        count=0;ready=false;
    }
    // Report whether this instance currently owns its native context.
    bool Active() const {return active;}
private:
    struct Block {unsigned char* address;unsigned long size;unsigned char* saved;};
    Block blocks[512];
    unsigned long count;
    bool active,ready;
    // Exchange active and saved bytes without double-swapping aliased ranges.
    void Swap() {
        for(unsigned long i=0;i<count;++i)
            for(unsigned long j=0;j<blocks[i].size;++j) {
                unsigned char t=blocks[i].address[j];
                blocks[i].address[j]=blocks[i].saved[j];blocks[i].saved[j]=t;
            }
    }
    CoOpContextBank(const CoOpContextBank&);
    CoOpContextBank& operator=(const CoOpContextBank&);
};
class CoOpContextScope
{
public:
    // Initialize co-op context scope with no active retail ownership.
    explicit CoOpContextScope(CoOpContextBank& b) : bank(b),entered(b.Enter()) {}
    // Release the scoped state owned by this object.
    ~CoOpContextScope() {if(entered) bank.Leave();}
    // Report the current entered state of the active roster and scoped retail globals.
    bool Entered() const {return entered;}
private:
    CoOpContextBank& bank;
    bool entered;
    CoOpContextScope(const CoOpContextScope&);
    CoOpContextScope& operator=(const CoOpContextScope&);
};
#endif
