"""Verify the retail instructions used by the playtest compatibility adapters."""
from pathlib import Path
import struct
repo=Path(__file__).resolve().parents[2]
exe=(repo.parent/'Spider-Man-decomp-baseline/SpideyPC.exe').read_bytes()
pe=struct.unpack_from('<I',exe,60)[0]
sections=pe+24+struct.unpack_from('<H',exe,pe+20)[0]
base=struct.unpack_from('<I',exe,pe+24+28)[0]
def read(address,size):
    rva=address-base
    for i in range(struct.unpack_from('<H',exe,pe+6)[0]):
        _,start,rawsize,pos=struct.unpack_from('<IIII',exe,sections+i*40+8)
        if start<=rva and rva+size<=start+rawsize:return exe[pos+rva-start:pos+rva-start+size]
    raise AssertionError(hex(address))
for site,target in ((0x4BC6C4,0x4E3940),(0x4F09F9,0x4C9130)):
    code=read(site,5)
    assert code[0]==0xE8 and site+5+struct.unpack_from('<i',code,1)[0]==target
for site,expected in {
    0x4F035F:'81 BA 1C 0E 00 00 00 00 00 20',
    0x44D373:'81 B9 1C 0E 00 00 00 00 00 10',
    0x4F044A:'C7 80 54 05 00 00 A0 0C 4F 00',
    0x4EF96C:'C7 80 54 05 00 00 70 0D 4F 00',
    0x4EEC31:'C7 80 54 05 00 00 70 0A 4F 00', # third restraint: airborne symbiote wrap
    0x4EEC1D:'80 CC 40', # wrap also owns damage-blocking flag
    0x4EEC7C:'81 CD 00 48 00 00', # wrap render flags
    0x4EF139:'8B 0D 38 90 6A 00 8B 86 24 04 00 00 89 81 54 05 00 00',
    0x4EF182:'8B 8E E4 03 00 00 E8 A3 97 F1 FF', # native effect Die
    0x4EF193:'89 9E E4 03 00 00 80 E6 B7', # effect pointer reset, wrap flags cleared
    0x4EF1AB:'68 B0 00 00 00', # wrap living release animation 176
    0x408949:'C6 46 37 01', # Die marks bit for later deletion
    0x4F096C:'89 8A 54 05 00 00',
    0x4F09E3:'C7 80 1C 0E 00 00 00 00 80 00',
    0x4EB83A:'8B 86 18 02 00 00 F6 C4 40 75 DA', # holding flag rejects damage
    0x4EB8E0:'66 29 86 E2 00 00', # native health subtraction
    0x4F0431:'68 B4 A3 55 00 80 CD 40', # hand hold owns the flag
    0x4EF84D:'80 CC 40', # symbiote hold owns the same flag
    0x4EF151:'80 E4 BF', # native hold release clears only this bit
    0x4F0A0A:'80 E5 BF',
    0x4EC233:'83 E1 F8', # web interruption leaves the holding bit intact
}.items():assert read(site,len(bytes.fromhex(expected)))==bytes.fromhex(expected),hex(site)
print('PASS retail synthesized snap hook, lizard/Venom held states, retained hand callback and native living-release animation/state')
