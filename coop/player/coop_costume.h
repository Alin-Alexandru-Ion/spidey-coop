// Co-op / player: Construct, update, carry, and recover each independent retail CPlayer.
#pragma once
#ifndef COOP_COSTUME_H
#define COOP_COSTUME_H

// Retail's costume viewer tests one bit per entry in the active save's
// unlock mask at 0x6828D8. Suit IDs are one greater than viewer entry IDs.
static unsigned gCoOpSelectedSuit=8; // P2's built-in Ben Reilly default
static const char* const gCoOpSuitNames[11]={"", "CLASSIC", "2099", "SYMBIOTE",
    "CAPTAIN UNIVERSE", "UNLIMITED", "BAG MAN", "SCARLET SPIDER",
    "DEFAULT", "QUICK CHANGE", "PETER PARKER"};
static const char* const gCoOpSuitResources[11]={"", "spidey", "sp2099", "spSymbi",
    "spUniv", "spUnlim", "spBagMan", "spScar", "spReilly", "spQuick", "spPark"};
// Update costume unlock mask on the selected CPlayer's independent lifecycle.
inline u32 CoOp_CostumeUnlockMask() {
    return *reinterpret_cast<const u32*>(0x006828D8);
}
// Update costume unlocked on the selected CPlayer's independent lifecycle.
inline bool CoOp_CostumeUnlocked(unsigned suit,u32 mask) {
    // Ben Reilly is P2's built-in default, not a separate unlock choice.
    return suit==1 || suit==8 || (suit>=2 && suit<=10 && (mask&(1u<<(suit-1)))!=0);
}
// Report whether costume validated for the selected CPlayer without overwriting the other player's state.
inline unsigned CoOp_CostumeValidated(unsigned suit,u32 mask) {
    return CoOp_CostumeUnlocked(suit,mask)?suit:8;
}
// Update costume next on the selected CPlayer's independent lifecycle.
inline unsigned CoOp_CostumeNext(unsigned suit,u32 mask) {
    for(unsigned step=1;step<=10;++step) {
        unsigned candidate=(suit+step-1)%10+1;
        if(CoOp_CostumeUnlocked(candidate,mask))return candidate;
    }
    return 8;
}
#endif
