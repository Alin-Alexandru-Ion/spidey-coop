#pragma once
#ifndef COOP_FRUSTUM_H
#define COOP_FRUSTUM_H
#ifndef COOP_RETAIL_ADDRESS
#define COOP_RETAIL_ADDRESS(x) (x)
#endif
// Retail arithmetic helpers only; no world or gameplay update.
inline void CoOp_UsePaneFrustum(const void* planes, void* rotation)
{
    memcpy(reinterpret_cast<void*>(COOP_RETAIL_ADDRESS(0x0065CEB8)),planes,0x50);
    typedef void (__cdecl *DataFn)(void*);
    typedef void (__cdecl *RotateFn)();
    reinterpret_cast<DataFn>(COOP_RETAIL_ADDRESS(0x0046D7B0))(rotation);
    for(int plane=0;plane<3;++plane)
    {
        reinterpret_cast<DataFn>(COOP_RETAIL_ADDRESS(0x0046D8A0))(
            reinterpret_cast<unsigned char*>(COOP_RETAIL_ADDRESS(0x0065CEB8))+plane*8);
        reinterpret_cast<RotateFn>(COOP_RETAIL_ADDRESS(0x0046DDF0))();
        reinterpret_cast<DataFn>(COOP_RETAIL_ADDRESS(0x0046DA10))(
            reinterpret_cast<unsigned char*>(COOP_RETAIL_ADDRESS(0x00628648))+plane*6);
        reinterpret_cast<DataFn>(COOP_RETAIL_ADDRESS(0x0046D8A0))(
            reinterpret_cast<unsigned char*>(COOP_RETAIL_ADDRESS(0x0065CED0))+plane*8);
        reinterpret_cast<RotateFn>(COOP_RETAIL_ADDRESS(0x0046DDF0))();
        reinterpret_cast<DataFn>(COOP_RETAIL_ADDRESS(0x0046DA10))(
            reinterpret_cast<unsigned char*>(COOP_RETAIL_ADDRESS(0x00628620))+plane*6);
    }
}
#endif
