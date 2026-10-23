#include <assert.h>
#include <math.h>
#include "coop/presentation/coop_partner_marker.h"

static CoOpMarkerCamera Camera(float cx,float cy) {
    CoOpMarkerCamera camera={0};
    camera.rotation[0][0]=camera.rotation[1][1]=camera.rotation[2][2]=4096;
    camera.focalX=camera.focalY=300;
    camera.centerX=cx;camera.centerY=cy;
    return camera;
}
static void Inside(const CoOpPartnerMarker& marker,const CoOpHudPane& pane) {
    assert(marker.valid);
    assert(marker.x>=pane.x && marker.x<=pane.x+pane.w);
    assert(marker.y>=pane.y && marker.y<=pane.y+pane.h);
}
static void AssertReadableLabel(const CoOpHudBuffer<128>& buffer,char digit) {
    const CoOpHudQuad& background=buffer.Get(buffer.Count()-2).quad;
    float unit=(background.v[1].x-background.v[0].x)/13.0f;
    assert(unit>=1.79f && unit<=2.61f);
    assert(background.v[0].color==0xFF111820);
    assert(buffer.Get(buffer.Count()-3).quad.v[0].color!=0xFF111820);
    unsigned char pixels[7][13]={0};
    for(unsigned i=0;i+3<buffer.Count();++i) {
        const CoOpHudQuad& q=buffer.Get(i).quad;
        if(q.v[0].color!=0xFFF8FBFF)continue;
        int col=(int)((q.v[0].x-background.v[0].x)/unit+.5f);
        int row=(int)((q.v[0].y-background.v[0].y)/unit+.5f)-1;
        assert(row>=0 && row<7 && col>=0 && col<13);
        pixels[row][col]=1;
    }
    const char* p[7]={"####.","#...#","#...#","####.","#....","#....","#...."};
    const char* one[7]={"..#..",".##..","..#..","..#..","..#..","..#..",".###."};
    const char* two[7]={".###.","#...#","....#","...#.","..#..",".#...","#####"};
    const char** numeral=digit=='1'?one:two;
    for(int row=0;row<7;++row)for(int col=0;col<13;++col) {
        bool expected=(col>=1 && col<=5 && p[row][col-1]=='#') ||
            (col>=7 && col<=11 && numeral[row][col-7]=='#');
        assert((pixels[row][col]!=0)==expected);
    }
}
int main() {
    CoOpHudPane left={0,0,320,480},right={320,0,320,480};
    CoOpHudPane top={0,0,640,240},bottom={0,240,640,240},full={0,0,640,480};
    double center[3]={0,0,400},nearby[3]={0,0,200},faraway[3]={0,0,1200};
    short pose[3][3]={{4096,0,0},{0,4096,0},{0,0,4096}};
    double head[3];CoOp_PartnerHeadTarget(center,pose,head);
    assert(head[0]==0 && head[1]==-64 && head[2]==400);
    pose[1][1]=-4096;pose[2][2]=-4096;
    CoOp_PartnerHeadTarget(center,pose,head);
    assert(head[1]==64); // inverted on a ceiling
    pose[0][1]=-4096;pose[1][1]=0;
    CoOp_PartnerHeadTarget(center,pose,head);
    assert(head[0]==64 && head[1]==0); // along a wall
    CoOpPartnerMarker p=CoOp_ProjectPartner(Camera(160,240),center,left);
    Inside(p,left);assert(p.onScreen);assert(fabs(p.x-160)<.01 && fabs(p.y-240)<.01);
    CoOpPartnerMarker nearMarker=CoOp_ProjectPartner(Camera(160,240),nearby,left);
    CoOpPartnerMarker farMarker=CoOp_ProjectPartner(Camera(160,240),faraway,left);
    assert(nearMarker.scale>p.scale && p.scale>farMarker.scale);
    assert(nearMarker.bodyPixels>p.bodyPixels && p.bodyPixels>farMarker.bodyPixels);
    p=CoOp_ProjectPartner(Camera(480,240),center,right);
    Inside(p,right);assert(p.onScreen && fabs(p.x-480)<.01);
    p=CoOp_ProjectPartner(Camera(320,120),center,top);
    Inside(p,top);assert(p.onScreen);
    p=CoOp_ProjectPartner(Camera(320,360),center,bottom);
    Inside(p,bottom);assert(p.onScreen && fabs(p.y-360)<.01);
    p=CoOp_ProjectPartner(Camera(320,240),center,full);
    Inside(p,full);assert(p.onScreen && fabs(p.x-320)<.01);
    // A quarter turn makes world -Z camera-right and world +X camera-forward.
    CoOpMarkerCamera turned=Camera(160,240);
    turned.rotation[0][0]=turned.rotation[2][2]=0;
    turned.rotation[2][0]=-4096;turned.rotation[0][2]=4096;
    double turnedForward[3]={400,0,0};
    p=CoOp_ProjectPartner(turned,turnedForward,left);
    Inside(p,left);assert(p.onScreen && fabs(p.x-160)<.01);
    double turnedRight[3]={400,0,-1000};
    p=CoOp_ProjectPartner(turned,turnedRight,left);
    Inside(p,left);assert(!p.onScreen && p.directionX>0);

    double leftTarget[3]={-1000,0,400},rightTarget[3]={1000,0,400};
    double upTarget[3]={0,-1000,400},downTarget[3]={0,1000,400};
    CoOpMarkerCamera cam=Camera(160,240);
    p=CoOp_ProjectPartner(cam,leftTarget,left);
    Inside(p,left);assert(!p.onScreen && p.directionX<0 && p.x<20);
    p=CoOp_ProjectPartner(cam,rightTarget,left);
    Inside(p,left);assert(!p.onScreen && p.directionX>0 && p.x>300);
    p=CoOp_ProjectPartner(cam,upTarget,left);
    Inside(p,left);assert(!p.onScreen && p.directionY<0 && p.y<20);
    p=CoOp_ProjectPartner(cam,downTarget,left);
    Inside(p,left);assert(!p.onScreen && p.directionY>0 && p.y>460);
    double nearTopHead[3]={0,-300,400};
    p=CoOp_ProjectPartner(cam,nearTopHead,left);
    Inside(p,left);assert(!p.onScreen && p.directionY<0);
    double behind[3]={0,0,-400};
    p=CoOp_ProjectPartner(cam,behind,left);
    Inside(p,left);assert(!p.onScreen);

    CoOpHudBuffer<128> buffer;
    p=CoOp_ProjectPartner(cam,nearby,left);
    CoOp_QueuePartnerMarker(buffer,p,left,1);
    assert(buffer.Count()>10 && buffer.Dropped()==0);
    AssertReadableLabel(buffer,'2');
    const CoOpHudQuad& label=buffer.Get(buffer.Count()-2).quad;
    const CoOpHudQuad& arrow=buffer.Get(buffer.Count()-1).quad;
    assert(label.depth>0 && label.depth<1);
    assert(buffer.Get(buffer.Count()-2).blend==1);
    assert(label.v[0].x<label.v[1].x && label.v[0].y==label.v[1].y);
    assert(label.v[1].x==label.v[2].x && label.v[1].y<label.v[2].y);
    assert(label.v[2].x>label.v[3].x && label.v[2].y==label.v[3].y);
    assert(label.v[3].x==label.v[0].x && label.v[3].y>label.v[0].y);
    assert(label.v[3].y<arrow.v[0].y); // P2 label above its pointer
    assert(arrow.v[0].y<arrow.v[2].y); // pointer aims down at the head
    assert(arrow.v[1].x-arrow.v[0].x>=10.0f); // visible, modestly wider head pointer
    assert(arrow.v[2].y+2.9f<p.y); // visible air gap above the head
    for(unsigned k=0;k<buffer.Count();++k) {
        const CoOpHudCommand& c=buffer.Get(k);
        assert(c.owner==1 && c.style==HUD_PROJECTED);
        for(unsigned j=0;j<4;++j) {
            assert(c.quad.v[j].x>=left.x && c.quad.v[j].x<=left.x+left.w);
            assert(c.quad.v[j].y>=left.y && c.quad.v[j].y<=left.y+left.h);
        }
    }
    buffer.Clear();
    p=CoOp_ProjectPartner(Camera(480,240),center,right);
    CoOp_QueuePartnerMarker(buffer,p,right,2);
    assert(buffer.Count()>10 && buffer.Dropped()==0);
    AssertReadableLabel(buffer,'1');
    const CoOpHudQuad& p1Label=buffer.Get(buffer.Count()-2).quad;
    const CoOpHudQuad& p1Arrow=buffer.Get(buffer.Count()-1).quad;
    assert(p1Label.v[3].y<p1Arrow.v[0].y);
    assert(p1Arrow.v[0].y<p1Arrow.v[2].y);
    assert(p1Arrow.v[2].y+2.9f<p.y);
    for(unsigned i=0;i<buffer.Count();++i) {
        const CoOpHudCommand& c=buffer.Get(i);
        assert(c.owner==2);
        for(unsigned j=0;j<4;++j)
            assert(c.quad.v[j].x>=right.x && c.quad.v[j].x<=right.x+right.w);
    }
    // At 1080p both vertical and horizontal split use the full display's
    // raster scale, even though their individual pane heights differ.
    CoOpHudPane hiVertical={0,0,960,1080},hiHorizontal={0,0,1920,540};
    double distant[3]={0,0,1600};
    CoOpMarkerCamera hiCamera=Camera(480,540);hiCamera.focalY=675;
    CoOpPartnerMarker hiV=CoOp_ProjectPartner(hiCamera,distant,hiVertical,1080);
    hiCamera.centerX=960;hiCamera.centerY=270;
    CoOpPartnerMarker hiH=CoOp_ProjectPartner(hiCamera,distant,hiHorizontal,1080);
    assert(hiV.onScreen && hiH.onScreen);
    assert(hiV.uiUnit>=4.49f && hiV.uiUnit<=5.86f);
    assert(fabs(hiV.uiUnit-hiH.uiUnit)<.01f);
    buffer.Clear();
    p=CoOp_ProjectPartner(cam,leftTarget,left);
    CoOp_QueuePartnerMarker(buffer,p,left,2);
    assert(buffer.Count()==1 && buffer.Get(0).owner==2);
    assert(buffer.Get(0).quad.depth>0 && buffer.Get(0).blend==1);
    assert(fabs(buffer.Get(0).quad.v[0].y-buffer.Get(0).quad.v[1].y)>16.0f);
    for(unsigned j=0;j<4;++j)
        assert(buffer.Get(0).quad.v[j].color==0xFFE95152);
    return 0;
}
