"""Confirm the installed retail instructions used by the narrow adapters."""
from pathlib import Path
from source_paths import header_path
import struct,re
repo=Path(__file__).resolve().parents[2]
data=(repo.parent/'Spider-Man-decomp-baseline/SpideyPC.exe').read_bytes()
pe=struct.unpack_from('<I',data,60)[0]
sections=pe+24+struct.unpack_from('<H',data,pe+20)[0]
base=struct.unpack_from('<I',data,pe+24+28)[0]
def read(address,size):
    rva=address-base
    for i in range(struct.unpack_from('<H',data,pe+6)[0]):
        _,start,rawsize,pos=struct.unpack_from('<IIII',data,sections+i*40+8)
        if start<=rva and rva+size<=start+rawsize:return data[pos+rva-start:pos+rva-start+size]
    raise AssertionError(hex(address))
header=(header_path(repo, "coop_armor_runtime.h")).read_text()
pattern=header.split('const u8 expected[]={',1)[1].split('};',1)[0]
expected=bytes(int(v,16) for v in re.findall(r'0x([0-9A-Fa-f]{2})',pattern))
assert len(expected)==0x474D86-0x474D5E
assert read(0x474D5E,len(expected))==expected
call=read(0x47E4DE,5)
assert call[0]==0xE8 and 0x47E4DE+5+struct.unpack_from('<i',call,1)[0]==0x460C10
# Native finishing call starts from CURRENT frame on every tick after timer 0.
assert read(0x47E4CE,0x10)==bytes.fromhex('75 13 0F BF 86 28 01 00 00 6A FF 50 6A 1E 8B CE')
# RunAnim unconditionally clears the fractional frame; UpdateFrame uses half
# of speed*delta. This is the precise recurrence exercised by the C++ test.
assert read(0x460CEC,9)==bytes.fromhex('66 C7 86 46 01 00 00 00 00')
assert read(0x460DB4,0xE)==bytes.fromhex('8B 81 4C 01 00 00 53 0F AF 81 80 00 00 00')
assert read(0x460DE1,2)==bytes.fromhex('D1 FE')
print('PASS installed armor selection branch and Rhino finishing-animation/frame-reset recurrence')
