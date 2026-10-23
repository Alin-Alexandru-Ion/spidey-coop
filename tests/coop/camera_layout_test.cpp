#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "router_test_types.h"
#include "coop/camera/coop_view.h"
struct Matrix {short m[3][3];i32 t[3];};
struct SCamera {CVector Position;struct {short vx,vy,vz;} Angles;Matrix Transform;};
static CoOpSplitLayout gCoOpLayout=COOP_SPLIT_LEFT_RIGHT;
static bool blocked;static unsigned rays;
static i32 __cdecl ClearRay(const CVector*,const CVector*,i32,i32) {++rays;return !blocked;}
typedef i32 (__cdecl *CoOpClearRayFn)(const CVector*,const CVector*,i32,i32);
#include "camera_layout_runtime.h"
int main() {
    CPlayer a,b;memset(&a,0,sizeof(a));memset(&b,0,sizeof(b));
    for(int orientation=0;orientation<3;++orientation) {
        SCamera ca;memset(&ca,0,sizeof(ca));ca.Transform.m[orientation][0]=4096;
        a.mPos.vx=40*4096;a.mPos.vy=50*4096;a.mPos.vz=60*4096;b.mPos=a.mPos;
        SCamera cb=ca;CoOp_CenterNarrowCamera(&ca,&a);CoOp_CenterNarrowCamera(&cb,&b);
        assert(!memcmp(&ca,&cb,sizeof(ca))); // floor/wall/ceiling basis treated symmetrically
        SCamera saved=ca;blocked=true;CoOp_CenterNarrowCamera(&ca,&a);
        assert(!memcmp(&ca,&saved,sizeof(ca)));blocked=false;
    }
    SCamera c;memset(&c,0,sizeof(c));c.Transform.m[0][0]=4096;a.mPos.vx=1000*4096;
    a.field_8EA=1;CoOp_CenterNarrowCamera(&c,&a);assert(c.Position.vx==0);a.field_8EA=0;
    CoOp_CenterNarrowCamera(&c,&a);assert(c.Position.vx==96 && c.Transform.t[0]==96);
    unsigned before=rays;gCoOpLayout=COOP_SPLIT_TOP_BOTTOM;
    SCamera saved=c;CoOp_CenterNarrowCamera(&c,&a);assert(rays==before && !memcmp(&saved,&c,sizeof(c)));
    puts("PASS actual camera centering: both players, three surface bases, blocked rays, bounded offset, untouched other layouts");return 0;
}
