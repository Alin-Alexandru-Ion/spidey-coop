#include <assert.h>
#include <stdio.h>
#include "coop/core/coop_context.h"
static void EarlyReturn(CoOpContextBank& bank,int* x) {
    CoOpContextScope outer(bank);assert(outer.Entered());
    CoOpContextScope nested(bank);assert(!nested.Entered());
    *x=27;
}
int main() {
    int a[6]={1,2,3,4,5,6},world=8;
    CoOpContextBank bank;
    assert(bank.Add(a+2,12));assert(bank.Add(a,12));assert(bank.Add(a+1,4));
    assert(bank.Capture());
    EarlyReturn(bank,a);
    assert(a[0]==1 && a[1]==2 && a[4]==5);
    {
        CoOpContextScope scope(bank);assert(scope.Entered());assert(a[0]==27);
        a[1]=99;world=88;
    }
    assert(a[1]==2 && world==88);
    a[0]=42;
    {CoOpContextScope scope(bank);assert(a[0]==27 && a[1]==99);}
    assert(a[0]==42 && a[1]==2);
    bank.Clear();bank.Clear();assert(!bank.Enter());
    puts("PASS: aliased/overlapping context ranges, independent state, nested rejection, early return restoration, shared world retained");
    return 0;
}
