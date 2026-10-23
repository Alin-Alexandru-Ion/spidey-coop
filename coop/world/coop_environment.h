// Co-op / world: Keep collision, objects, and section activation valid around both players.
#pragma once
#ifndef COOP_ENVIRONMENT_H
#define COOP_ENVIRONMENT_H
// Own only visibility changes made by the authored box command. Never clear
// arbitrary hidden objects, unload resources, or mutate the collision-zone map.
template<class Item, class Vector> class CoOpEnvironment
{
    struct Entry
    {
        Item* item;
        Vector min,max,origin;
        bool fixedArea, lastHidden;unsigned sourceSlot;
    };
    enum { CAPACITY=4096 };
    Entry entries[CAPACITY];
    unsigned int count;
    unsigned int held;
    // Track contains while retaining geometry needed by either player.
    static bool Contains(const Vector& p,const Vector& min,const Vector& max)
    {
        return p.vx>=min.vx && p.vx<=max.vx &&
            p.vy>=min.vy && p.vy<=max.vy && p.vz>=min.vz && p.vz<=max.vz;
    }
    // Track valid box while retaining geometry needed by either player.
    static bool ValidBox(const Vector& min,const Vector& max)
    {
        // Exclude whole-world reset commands and malformed coordinates.
        return min.vx<=max.vx && min.vy<=max.vy && min.vz<=max.vz &&
            (double)max.vx-min.vx<1073741824.0 &&
            (double)max.vy-min.vy<1073741824.0 &&
            (double)max.vz-min.vz<1073741824.0;
    }
    // Track remove while retaining geometry needed by either player.
    void Remove(unsigned int i) {entries[i]=entries[--count];}
public:
    // Initialize co-op environment with no active retail ownership.
    CoOpEnvironment():count(0),held(0) {}
    // Return tracked from the union of both players' active regions.
    unsigned int Tracked() const {return count;}
    // Return held from the union of both players' active regions.
    unsigned int Held() const {return held;}
    // Release owned resources and reset the union of both players' active regions to its inactive state.
    void Clear(bool restore)
    {
        if(restore)
            for(unsigned int i=0;i<count;++i)
                if(!entries[i].lastHidden && !(entries[i].item->mFlags&1))
                    entries[i].item->mFlags|=1;
        count=held=0;
    }
    // Call after the original visibility command so retail still establishes
    // its authoritative state. All coordinates are retail fixed-point units.
    void Observe(Item* head,const Vector& min,const Vector& max,
        bool visible,bool inside,const Vector& primary,unsigned sourceSlot=0)
    {
        for(Item* item=head;item;item=static_cast<Item*>(item->mNextItem))
        {
            const Vector& p=item->mPos;
            // Mirror retail's unusual outside-box test exactly.
            bool affected=inside ? Contains(p,min,max) :
                ((p.vx<min.vx || p.vx>max.vx) &&
                 (p.vy<min.vy || p.vy>max.vy) &&
                 (p.vz<min.vz || p.vz>max.vz));
            if(!affected) continue;
            unsigned int i;
            for(i=0;i<count && entries[i].item!=item;++i) {}
            if(visible || !ValidBox(min,max) || !(item->mFlags&1))
            {if(i<count) Remove(i);continue;}
            if(i==count)
            {if(count==CAPACITY) continue; ++count;}
            entries[i].item=item;entries[i].min=min;entries[i].max=max;
            entries[i].origin=primary;entries[i].fixedArea=inside;entries[i].sourceSlot=sourceSlot;
            entries[i].lastHidden=true;
        }
    }
    // Track update while retaining geometry needed by either player.
    void Update(const Vector* secondary,const Vector* primary=0)
    {
        held=0;
        for(unsigned int i=0;i<count;)
        {
            Entry& entry=entries[i];
            bool actual=(entry.item->mFlags&1)!=0;
            // An unrelated script/destruction changed this object: relinquish
            // ownership rather than undoing that change on the following tick.
            if(actual!=entry.lastHidden) {Remove(i);continue;}
            bool keep=false;
            const Vector* partner=entry.sourceSlot==1?primary:secondary;
            if(partner)
            {
                if(entry.fixedArea) keep=Contains(*partner,entry.min,entry.max);
                else
                {
                    // Translate the original P1 box to P2 without integer
                    // overflow and without enlarging the region between them.
                    double x=(double)entry.item->mPos.vx-partner->vx+entry.origin.vx;
                    double y=(double)entry.item->mPos.vy-partner->vy+entry.origin.vy;
                    double z=(double)entry.item->mPos.vz-partner->vz+entry.origin.vz;
                    keep=x>=entry.min.vx && x<=entry.max.vx &&
                        y>=entry.min.vy && y<=entry.max.vy &&
                        z>=entry.min.vz && z<=entry.max.vz;
                }
            }
            if(keep) {entry.item->mFlags&=~1; ++held;}
            else entry.item->mFlags|=1;
            entry.lastHidden=!keep;
            ++i;
        }
    }
};
#endif
