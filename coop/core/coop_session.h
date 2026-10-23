// Co-op / core: Own the two-player roster and scoped compatibility state for retail globals.
#pragma once
#ifndef COOP_SESSION_H
#define COOP_SESSION_H
// Each participant owns its inventory and health across native level lifetimes.
// Retail checkpoint globals are construction inputs, not the co-op roster.
struct CoOpPlayerCarry {
    bool valid;
    int webbing,cartridges,armorHealth,health,maxHealth;
    bool armor;
    // Initialize co-op player carry with no active retail ownership.
    CoOpPlayerCarry():valid(false),webbing(0),cartridges(0),armorHealth(0),health(0),maxHealth(100),armor(false) {}
    // Release owned resources and reset the active roster and scoped retail globals to its inactive state.
    void Clear(){valid=false;webbing=cartridges=armorHealth=health=0;maxHealth=100;armor=false;}
    // 5EC is remaining armor health, not the selected web weapon.
    template<class Player> void Capture(const Player* p) {
        if(!p){Clear();return;}
        valid=true;maxHealth=p->mMaxHealth>0?p->mMaxHealth:100;
        health=p->mHealth;webbing=p->mWebbing;cartridges=p->field_5D8;
        armorHealth=p->field_5EC;armor=p->field_5E9!=0;
        if(health<=0)ResetAfterDeath();
    }
    // Reset only the dead player's carry values to the co-op respawn defaults.
    void ResetAfterDeath(){valid=true;health=maxHealth;webbing=4096;cartridges=7;armorHealth=0;armor=false;}
    template<class Player> void Apply(Player* p) const {
        if(!valid || !p)return;
        p->mHealth=health;p->mMaxHealth=maxHealth;p->mWebbing=webbing;
        p->field_5D8=cartridges;p->field_5EC=armorHealth;p->field_5E9=armor;
    }
};
// Session intent survives destruction of the level's player/camera objects.
// A successful join consumes the request, so death or manual drop-out does not
// continuously recreate P2. The next level/retry arms it again.
struct CoOpSession {
    bool enabled,pending;
    CoOpPlayerCarry carry,primaryCarry;
    // Initialize co-op session with no active retail ownership.
    CoOpSession():enabled(false),pending(false) {}
    // Update select in the authoritative two-player roster.
    void Select(bool coop) {enabled=coop;pending=coop;carry.Clear();primaryCarry.Clear();}
    // Update level ended in the authoritative two-player roster.
    void LevelEnded() {pending=enabled;}
    // Report the current joined state of the active roster and scoped retail globals.
    void Joined() {pending=false;}
};
#endif
