// Co-op / world: Keep collision, objects, and section activation valid around both players.
#pragma once
#ifndef COOP_ACTIVATION_H
#define COOP_ACTIVATION_H
// Distance to the union of the original-sized activation regions. Deliberately
// reuse retail's distance function rather than substituting Euclidean distance.
template<class Vector, class Distance>
// Measure an item's activation distance from the nearest living player.
inline int CoOp_NearestActivationDistance(Vector* primary, Vector* item,
    Vector* secondary, Distance distance)
{
    int result=distance(primary,item);
    if(secondary)
    {
        int other=distance(secondary,item);
        if(other<result) result=other;
    }
    return result;
}
// Retail culling writes 0x8000 into CItem::mFlags. After both views, retain the
// union of visibility, preserving all unrelated flags. Never gate simulation on
// this flag: it is a camera result, not an activation/lifetime state.
template<class Item> class CoOpVisibilityUnion
{
    struct Entry { Item* item; unsigned short culled; };
    Entry* entries;
    unsigned int count;
public:
    // Initialize co-op visibility union with no active retail ownership.
    CoOpVisibilityUnion(Item* head) : entries(0), count(0)
    {
        Item* item;
        for(item=head;item;item=static_cast<Item*>(item->mNextItem)) ++count;
        if(!count) return;
        entries=new Entry[count];
        unsigned int index=0;
        for(item=head;item;item=static_cast<Item*>(item->mNextItem),++index)
        {
            entries[index].item=item;entries[index].culled=item->mFlags&0x8000;
            // P1's camera may have culled this actor. Re-evaluate it for P2;
            // the destructor restores the union after the second draw.
            item->mFlags&=~0x8000;
        }
    }
    // Release the scoped state owned by this object.
    ~CoOpVisibilityUnion()
    {
        for(unsigned int index=0;index<count;++index)
        {
            Item* item=entries[index].item;
            item->mFlags=(item->mFlags&0x7FFF)|
                (item->mFlags&entries[index].culled&0x8000);
        }
        delete[] entries;
    }
private:
    CoOpVisibilityUnion(const CoOpVisibilityUnion&);
    CoOpVisibilityUnion& operator=(const CoOpVisibilityUnion&);
};
#endif
