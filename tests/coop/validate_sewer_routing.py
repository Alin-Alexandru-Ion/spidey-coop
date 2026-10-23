"""Validate the installed AI/setter boundaries and authored sewer camera exits."""
from pathlib import Path
import struct,zlib
repo=Path(__file__).resolve().parents[2]
exe=(repo.parent/'Spider-Man-decomp-baseline/SpideyPC.exe').read_bytes()
pe=struct.unpack_from('<I',exe,60)[0];sections=pe+24+struct.unpack_from('<H',exe,pe+20)[0]
base=struct.unpack_from('<I',exe,pe+24+28)[0]
def read(address,size):
    rva=address-base
    for i in range(struct.unpack_from('<H',exe,pe+6)[0]):
        _,start,rawsize,pos=struct.unpack_from('<IIII',exe,sections+i*40+8)
        if start<=rva and rva+size<=start+rawsize:return exe[pos+rva-start:pos+rva-start+size]
    raise AssertionError(hex(address))
assert struct.unpack('<I',read(0x53B9B4,4))[0]==0x451350
for site,target in ((0x4E274C,0x4167F0),(0x4E23F8,0x4B9E10),(0x4E64A2,0x452C30),(0x4521EB,0x452C30)):
    code=read(site,5)
    assert code[0]==0xE8 and site+5+struct.unpack_from('<i',code,1)[0]==target
# Native lizard melee sends damage to MechList's virtual Hit, inside routed AI.
assert read(0x44D310,6)==bytes.fromhex('8B 0D 38 90 6A 00')
assert read(0x44D340,3)==bytes.fromhex('FF 52 0C')
data=(repo.parent/'Spider-Man-decomp-baseline/data.pkr').read_bytes()
off=struct.unpack_from('<I',data,4)[0];_,dirs,files=struct.unpack_from('<III',data,off);off+=12+dirs*40
checked=0
for i in range(files):
    name,_,compression,pos,_,packed=struct.unpack_from('<32sIiIII',data,off+i*52)
    name=name.split(b'\0')[0].decode().lower()
    if name not in ('l5a3_t.trg','l5a4_t.trg'):continue
    raw=data[pos:pos+packed];raw=zlib.decompress(raw) if compression==2 else raw
    count=struct.unpack_from('<I',raw,8)[0];offsets=list(struct.unpack_from('<'+'I'*count,raw,12))+[len(raw)]
    for node in ((130,158) if name=='l5a3_t.trg' else (123,148)):
        start,end=offsets[node:node+2];kind,links=struct.unpack_from('<HH',raw,start);assert kind==6
        begin=((start+4+links*2+3)&~3)+4
        assert struct.unpack_from('<4H',raw,begin)==(160,3,187,0)
        checked+=1
assert checked==4
print('PASS installed LizMan AI/melee target and authored camera setters; four native sewer release nodes')
