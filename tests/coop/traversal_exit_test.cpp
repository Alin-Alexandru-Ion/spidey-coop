#include <assert.h>
#include <stdio.h>
#include "coop/script/coop_traversal_exit.h"
static const unsigned short route[]={195,30,199,0,17,30,3,10,60,520,1,54,20,3,16,2,20,3,16,2,20,3,16,2,20,3,16,2,20,3,16,2,20,3,16,2,20,3,16,2,20,3,16,2,20,3,16,2,20,3,16,2,20,3,16,2,20,3,16,2,400,1,276,0,2,277,0,1,54,110,3,9,150,123,3,16,3,20,3,7,60,40,3,10,60,0,255,65535};
int main(){unsigned n=sizeof(route)/sizeof(route[0]);
 assert(CoOp_TraversalSnapNode(54,route,n,route+69)==277);
 for(unsigned i=0;i<n;++i)if(i!=69)assert(CoOp_TraversalSnapNode(54,route,n,route+i)==54);
 assert(CoOp_TraversalSnapNode(276,route,n,route+69)==276);
 unsigned short altered[128];for(unsigned k=0;k<n;++k)altered[k]=route[k];altered[5]++;
 assert(CoOp_TraversalSnapNode(54,altered,n,altered+69)==54);
 assert(CoOp_TraversalSnapNode(54,route,n-1,route+69)==54);
 puts("PASS actual Venom traversal route: final snap uses authored run exit 277, entry snaps/camera markers and changed routes untouched");return 0;}
