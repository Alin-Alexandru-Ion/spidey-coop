// Co-op / script: Coordinate authored motion and cinematics with co-op control handoffs.
#pragma once
// L5A1 node 71 runs the actor to exit marker 277, then erroneously snaps
// back to entry marker 54. Correct only that final snap in this exact route.
// Other snaps, other banks and camera markers retain retail behavior.
static int CoOp_TraversalSnapNode(int node,const unsigned short* commands,
    unsigned words,const unsigned short* cursor) {
    static const unsigned short expected[]={195,30,199,0,17,30,3,10,60,520,1,54,20,3,16,2,20,3,16,2,20,3,16,2,20,3,16,2,20,3,16,2,20,3,16,2,20,3,16,2,20,3,16,2,20,3,16,2,20,3,16,2,20,3,16,2,20,3,16,2,400,1,276,0,2,277,0,1,54,110,3,9,150,123,3,16,3,20,3,7,60,40,3,10,60,0,255,65535};
    if(node!=54 || !commands || words!=sizeof(expected)/sizeof(expected[0]) ||
       cursor!=commands+69)return node;
    for(unsigned i=0;i<words;++i)if(commands[i]!=expected[i])return node;
    return 277;
}
