// Co-op / menu: Expose co-op configuration and preview through retail menu flows.
#pragma once
static CDummy* gCoOpMenuSpidey;
static CDummy* gCoOpMenuPartner;
static CDummy* __fastcall CoOp_MenuDummyCtor(CDummy* self,void*,const char* model,
    int type,int scale,int y,int anim,u16* a,u16* b,u16* c,int d,int e,int f,int g) {
    typedef CDummy* (__fastcall *Fn)(CDummy*,void*,const char*,int,int,int,int,u16*,u16*,u16*,int,int,int,int);
    Fn ctor=reinterpret_cast<Fn>(0x00490DF0);
    gCoOpMenuSpidey=ctor(self,0,model,type,scale,y,anim,a,b,c,d,e,f,g);
    if(gCoOpMainSubmenu==2) {
        void* memory=reinterpret_cast<void* (__cdecl*)(u32)>(0x00455390)(sizeof(CDummy));
        if(memory) {
            // The native CDummy constructor spools the model name it receives.
            // Keep P1's shell preview and suit untouched while constructing P2.
            unsigned suit=CoOp_CostumeValidated(gCoOpSelectedSuit,CoOp_CostumeUnlockMask());
            i32 savedSuit=*reinterpret_cast<i32*>(0x005559DC);
            *reinterpret_cast<i32*>(0x005559DC)=suit;
            gCoOpMenuPartner=ctor(static_cast<CDummy*>(memory),0,gCoOpSuitResources[suit],
                type,scale,y,anim,a,b,c,d,e,f,g);
            *reinterpret_cast<i32*>(0x005559DC)=savedSuit;
            // Start one third into its own animation, then let its native AI
            // advance independently. No replay of the first actor's pose/clock.
            if(gCoOpMenuPartner) {
                int frames=gCoOpMenuPartner->mNumFrames;
                if(frames>2) reinterpret_cast<void (__fastcall*)(CDummy*,void*,int,int,int)>(0x00460C10)
                    (gCoOpMenuPartner,0,gCoOpMenuPartner->mAnim,frames/3,-1);
            }
        }
    }
    return gCoOpMenuSpidey;
}
// Draw the selected P2 costume behind the regular menu Spidey.
static void __cdecl CoOp_MenuDrawPreview(void* selected) {
    typedef void (__cdecl *Fn)(void*);Fn render=reinterpret_cast<Fn>(0x004739A0);
    if(gCoOpMainSubmenu!=2 || !gCoOpMenuSpidey) {render(selected);return;}
    render(gCoOpMenuSpidey);
    if(gCoOpMenuPartner) {
        gCoOpMenuPartner->mPos=gCoOpMenuSpidey->mPos;
        gCoOpMenuPartner->mPos.vx+=120*4096;
        gCoOpMenuPartner->mPos.vz+=130*4096; // menu camera looks along positive Z
        gCoOpMenuPartner->mAngles=gCoOpMenuSpidey->mAngles;
        render(gCoOpMenuPartner);
    }
}
// Update menu update effects in the native co-op menu flow.
static void __cdecl CoOp_MenuUpdateEffects() {
    if(gCoOpMenuPartner) {
        typedef void (__fastcall *Fn)(CDummy*,void*);
        reinterpret_cast<Fn>((*reinterpret_cast<u32**>(gCoOpMenuPartner))[2])(gCoOpMenuPartner,0);
    }
    reinterpret_cast<void (__cdecl*)()>(0x00411B30)();
}
// Free the second menu actor when the co-op submenu closes.
static void CoOp_MenuDestroyPreview() {
    if(gCoOpMenuPartner) {
        typedef void (__fastcall *Fn)(CDummy*,void*,int);
        reinterpret_cast<Fn>((*reinterpret_cast<u32**>(gCoOpMenuPartner))[0])(gCoOpMenuPartner,0,1);
    }
    gCoOpMenuPartner=gCoOpMenuSpidey=0;
}
