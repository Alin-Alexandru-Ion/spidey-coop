"""Verify life hook instruction boundaries against the supported retail EXE."""
from validate_menu_hooks import read, image, pe, optional_size
import struct
assert read(0x4BD7E0,8)==bytes.fromhex("568bf1e8182ffaff")
assert read(0x4B28AE,5)==bytes.fromhex("a3b0b15f00")
for address,target in [(0x4B28FF,0x4BDFF0),(0x4B5EF3,0x4BDFF0),(0x4BDB10,0x4BDFF0),(0x4E147B,0x4BDFF0),(0x455BD3,0x460700)]:
    assert read(address,1)==b"\xe8"
    assert address+5+struct.unpack("<i",read(address+1,4))[0]==target
print("PASS supported EXE life hooks: virtual Die, combat death calls, game-over check and legacy fall branch")

# Audit all direct CALLs in executable code, rather than only validating a
# hand-picked list (the missing fourth caller caused the previous regression).
callers=set()
for section in range(struct.unpack_from("<H",image,pe+6)[0]):
    header=pe+24+optional_size+section*40
    vs,va,size,offset=struct.unpack_from("<IIII",image,header+8)
    flags=struct.unpack_from("<I",image,header+36)[0]
    if not flags&0x20000000:continue
    code=image[offset:offset+size]
    for i in range(len(code)-4):
        address=0x400000+va+i
        if code[i]==0xE8 and address+5+struct.unpack_from("<i",code,i+1)[0]==0x4BDFF0:callers.add(address)
assert callers=={0x4B28FF,0x4B5EF3,0x4BDB10,0x4E147B}, callers
assert read(0x452C19,1)==b"\xe8"
assert 0x452C1E+struct.unpack("<i",read(0x452C1A,4))[0]==0x4DE8D0
print("PASS exhaustive death-call audit includes script Die; native collision trigger call verified")
