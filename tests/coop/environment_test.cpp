#include <assert.h>
#include <stdio.h>
#include "coop/world/coop_environment.h"
struct V {int vx,vy,vz;};
struct Item {unsigned short mFlags;V mPos;Item* mNextItem;};
static CoOpEnvironment<Item,V> environment;
int main()
{
    V origin={0,0,0},min={90,90,90},max={110,110,110},p2={100,100,100};
    Item item={0x1001,{100,100,100},0};
    environment.Observe(&item,min,max,false,true,origin);
    environment.Update(&p2);
    assert(item.mFlags==0x1000 && environment.Held()==1);
    p2.vx=120;environment.Update(&p2);assert(item.mFlags==0x1001);
    p2.vx=100;environment.Update(&p2);assert(item.mFlags==0x1000);
    item.mFlags|=1;environment.Update(&p2); // destruction wins
    assert(item.mFlags==0x1001 && environment.Tracked()==0);
    item.mFlags=1;min.vx=min.vy=min.vz=-10;max.vx=max.vy=max.vz=10;
    environment.Observe(&item,min,max,false,false,origin);
    environment.Update(&p2);assert(!(item.mFlags&1));
    environment.Update(0);assert(item.mFlags&1);
    environment.Update(&p2);environment.Clear(true);assert(item.mFlags&1);
    // Visible commands relinquish ownership and stay visible after P2 leaves.
    item.mFlags=1;environment.Observe(&item,min,max,false,false,origin);
    item.mFlags=0;environment.Observe(&item,min,max,true,false,origin);
    environment.Update(0);assert(item.mFlags==0);
    // P2 enters a secret room; stationary P1 still needs the exterior box.
    environment.Clear(false);item.mFlags=1;
    min.vx=min.vy=min.vz=90;max.vx=max.vy=max.vz=110;
    V cabin={500,500,500},outside={100,100,100};
    environment.Observe(&item,min,max,false,true,cabin,1);
    environment.Update(&cabin,&outside);assert(!(item.mFlags&1));
    outside.vx=120;environment.Update(&cabin,&outside);assert(item.mFlags&1);
    puts("PASS fixed authored areas, translated P2 box, departure, destruction, visibility ownership");
    return 0;
}
