// Co-op / player: Construct, update, carry, and recover each independent retail CPlayer.
#pragma once
struct CoOpLifeSlot {
    bool waiting,recover,revive;
    // Initialize co-op life slot with no active retail ownership.
    CoOpLifeSlot():waiting(false),recover(false),revive(false) {}
};
struct CoOpLife {
    CoOpLifeSlot slot[2];
    bool active;
    // Initialize co-op life with no active retail ownership.
    CoOpLife():active(false) {}
    // Release owned resources and reset the selected CPlayer without overwriting the other player's state to its inactive state.
    void Clear(){active=false;slot[0]=CoOpLifeSlot();slot[1]=CoOpLifeSlot();}
    // Update both dead on the selected CPlayer's independent lifecycle.
    bool BothDead() const{return active && slot[0].waiting && slot[1].waiting;}
    // Update waiting on the selected CPlayer's independent lifecycle.
    bool Waiting(unsigned i) const{return active && i<2 && slot[i].waiting;}
    // Update die on the selected CPlayer's independent lifecycle.
    void Die(unsigned i){if(i<2 && !slot[i].waiting){slot[i].waiting=true;slot[i].recover=false;slot[i].revive=false;}}
    // Update fall on the selected CPlayer's independent lifecycle.
    int Fall(unsigned i,int health,int maximum) {
        if(i>=2 || slot[i].waiting || slot[i].recover) return health;
        int cost=((maximum>0?maximum:100)+3)/4;
        health=health>cost?health-cost:0;
        if(!health) Die(i);else slot[i].recover=true;
        return health;
    }
    // Update checkpoint on the selected CPlayer's independent lifecycle.
    void Checkpoint(){if(active && !BothDead()) for(unsigned i=0;i<2;++i) if(slot[i].waiting)slot[i].revive=true;}
    // Update recovered on the selected CPlayer's independent lifecycle.
    void Recovered(unsigned i){slot[i]=CoOpLifeSlot();}
};
