// Co-op / world: Keep collision, objects, and section activation valid around both players.
#pragma once
// Recognize authored pulse chains that lead to a native Die command.
// Only command points and linked actor nodes participate; no raw word scanning.
struct CoOpFallNode {unsigned type,actorType;const unsigned short* links;const unsigned short* commands;};
typedef bool (*CoOpFallRead)(unsigned,CoOpFallNode&,void*);
// Identify a linked fall/death trigger without consuming unrelated scripts.
static bool CoOp_FallChain(unsigned root,CoOpFallRead read,void* context) {
    unsigned queue[128],count=1;queue[0]=root;
    for(unsigned at=0;at<count;++at) {
        CoOpFallNode n;if(!read(queue[at],n,context))continue;
        if(n.type==6) {
            const unsigned short* c=n.commands;if(!c)continue;
            if(c[0]==152 && c[1]==0xFFFF)return true; // retail Die
            if(c[0]==134 && c[1]==1)c+=2; // one pulse, no condition
            if(c[0]!=3 || c[1]!=0xFFFF)continue; // pure SendPulse only
        } else if(n.type!=1 || n.actorType!=203 || at==0)continue;
        if(!n.links || n.links[0]>127)continue;
        for(unsigned k=1;k<=n.links[0];++k) {
            unsigned child=n.links[k],j;for(j=0;j<count && queue[j]!=child;++j){}
            if(j==count && count<128)queue[count++]=child;
        }
    }
    return false;
}
