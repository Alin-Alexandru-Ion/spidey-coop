// Co-op / world: Keep collision, objects, and section activation valid around both players.
#pragma once
#ifndef COOP_WORLD_TRACE_H
#define COOP_WORLD_TRACE_H
#include <stdio.h>
#include <string.h>
// Passive snapshots only. Never dereference an object retained from an older
// snapshot: disappearance may mean the retail allocator already freed it.
template<class Item, unsigned Capacity=8192> class CoOpWorldTrace
{
    struct Entry {
        const Item* key;
        unsigned stamp;
        unsigned short flags;
        bool alive;
    };
    struct Event {
        unsigned phase, kind;
        const Item* key;
        unsigned short before, after;
        long x,y,z;
    };
    struct Summary {
        unsigned count, hidden, noCollision, excluded, culled, hash, resources, grid;
        bool incomplete;
    };
    Entry entries[Capacity];
    Event events[128];
    Summary summaries[6];
    unsigned stamp, usedEvents, dropped;
    bool initialized;
    // Track record while retaining geometry needed by either player.
    void Record(unsigned phase,unsigned kind,const Item* item,unsigned short before,
        unsigned short after,bool position) {
        if(usedEvents==128) {++dropped;return;}
        Event& e=events[usedEvents++];
        e.phase=phase;e.kind=kind;e.key=item;e.before=before;e.after=after;
        e.x=e.y=e.z=0;
        if(position) {e.x=item->mPos.vx/4096;e.y=item->mPos.vy/4096;e.z=item->mPos.vz/4096;}
    }
public:
    // Initialize co-op world trace with no active retail ownership.
    CoOpWorldTrace() {Clear();}
    // Release owned resources and reset the union of both players' active regions to its inactive state.
    void Clear() {
        memset(entries,0,sizeof(entries));memset(summaries,0,sizeof(summaries));
        stamp=usedEvents=dropped=0;initialized=false;
    }
    // Merge overlapping ranges and save one independent copy of their bytes.
    void Capture(Item* head,unsigned phase,unsigned resources,unsigned grid) {
        if(phase>=6) return;
        if(++stamp==0) {Clear();++stamp;}
        Summary s;memset(&s,0,sizeof(s));s.hash=2166136261u;
        s.resources=resources;s.grid=grid;
        Item* item=head;
        for(;item && s.count<Capacity;item=static_cast<Item*>(item->mNextItem)) {
            ++s.count;
            unsigned short flags=item->mFlags;
            if(flags&1) ++s.hidden;
            if(flags&0x20) ++s.noCollision;
            if(flags&0x1000) ++s.excluded;
            if(flags&0x8000) ++s.culled;
            s.hash=(s.hash^(unsigned long)item)*16777619u;
            s.hash=(s.hash^(flags&0x7fff))*16777619u;
            s.hash=(s.hash^item->mPos.vx)*16777619u;
            s.hash=(s.hash^item->mPos.vy)*16777619u;
            s.hash=(s.hash^item->mPos.vz)*16777619u;
            unsigned slot=((unsigned long)item>>4)%Capacity,probe=0;
            for(;probe<Capacity;++probe,slot=(slot+1)%Capacity)
                if(!entries[slot].key || entries[slot].key==item) break;
            if(probe==Capacity) {s.incomplete=true;continue;}
            Entry& e=entries[slot];
            if(initialized) {
                if(!e.alive) Record(phase,1,item,0,flags,true);
                else if((e.flags^flags)&0x1021)
                    Record(phase,2,item,e.flags,flags,true);
            }
            e.key=item;e.flags=flags;e.stamp=stamp;e.alive=true;
        }
        if(item) s.incomplete=true; // includes corrupt/cyclic or oversized lists
        if(!s.incomplete) for(unsigned i=0;i<Capacity;++i) {
            Entry& e=entries[i];
            if(e.alive && e.stamp!=stamp) {
                if(initialized) Record(phase,3,e.key,e.flags,0,false);
                e.alive=false;
            }
        }
        summaries[phase]=s;initialized=true;
    }
    // Track write while retaining geometry needed by either player.
    void Write(FILE* log) {
        static const char* phases[]={"before-P1","after-P1","after-P2-and-camera",
            "before-render","after-P1-render","after-P2-render"};
        for(unsigned i=0;i<6;++i) {
            const Summary& s=summaries[i];
            fprintf(log,"world %s: count=%u hidden1=%u noCollision20=%u excluded1000=%u culled8000=%u hash=%08x resources=%08x grid=%08x incomplete=%d\n",
                phases[i],s.count,s.hidden,s.noCollision,s.excluded,s.culled,
                s.hash,s.resources,s.grid,(int)s.incomplete);
        }
        for(unsigned j=0;j<usedEvents;++j) {
            const Event& e=events[j];
            fprintf(log,"world-change %s kind=%u item=%p flags=%04x->%04x xyz=%ld,%ld,%ld\n",
                phases[e.phase],e.kind,e.key,e.before,e.after,e.x,e.y,e.z);
        }
        fprintf(log,"world-events: dropped=%u (kind 1=added 2=flags 3=absent; absent pointers are not dereferenced)\n",dropped);
        usedEvents=dropped=0;
    }
    // Return events from the union of both players' active regions.
    unsigned Events() const {return usedEvents;}
    // Return dropped from the union of both players' active regions.
    unsigned Dropped() const {return dropped;}
};
#endif
