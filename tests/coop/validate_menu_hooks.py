"""Read-only verification of the main-menu patch operands against the supported EXE."""
from pathlib import Path
from source_paths import header_path
import hashlib
import re
import struct
repo = Path(__file__).resolve().parents[2]
source = (header_path(repo, "coop_menu_runtime.h")).read_text()
image = (repo.parent / "Spider-Man-decomp-baseline" / "SpideyPC.exe").read_bytes()
assert hashlib.sha256(image).hexdigest() == "d55a0bb0e920c497ce1ca76f08ed2e62feefcb6ff3c2901c0d59890f099ba93c"
pe = struct.unpack_from("<I", image, 60)[0]
optional_size = struct.unpack_from("<H", image, pe+20)[0]
def read(address, size):
    for i in range(struct.unpack_from("<H", image, pe+6)[0]):
        vs, va, rs, raw = struct.unpack_from("<IIII", image, pe+24+optional_size+i*40+8)
        if va <= address-0x400000 < va+max(vs, rs):
            offset = raw+address-0x400000-va
            return image[offset:offset+size]
    raise AssertionError(hex(address))
def numbers(name):
    return [int(x,16) for x in re.findall("0x[0-9A-Fa-f]+", source.split(name+"[]={")[1].split("};")[0])]
for address, value in zip(numbers("gCoOpMainPointerSites"), numbers("gCoOpMainPointerValues")):
    assert struct.unpack("<I", read(address,4))[0] == value, hex(address)
for address, value in [(0x4939D9,0x552BF4),(0x493EF0,0x552BE8)]:
    assert struct.unpack("<I", read(address,4))[0] == value
for address, value in [(0x49386E,8),(0x493D73,8),(0x493D89,4),(0x493D08,0x75)]:
    assert read(address,1) == bytes([value])
for address, target in [(0x4A202F,0x493990),(0x493E1F,0x50C180),(0x493E56,0x50C180),(0x493DBE,0x50C470),(0x493B3E,0x490DF0),(0x493C15,0x4739A0),(0x494142,0x411B30),(0x4E2515,0x4B97D0)]:
    assert read(address,1) == b"\xe8"
    assert address+5+struct.unpack("<i",read(address+1,4))[0] == target
assert read(0x493C12,2) == bytes.fromhex("7409")
assert read(0x493EFA,5) == bytes.fromhex("6a006a0068")
# Verify the bridge's stack-slot contract and the preview update's stack argument.
assert read(0x493EB5,4) == bytes.fromhex("8b6c2440")
assert read(0x494133,1) == b"\x55"
assert read(0x49425B,4) == bytes.fromhex("8b442440")
# CDummy's model argument is passed to native Spool_PSX. The rear preview can
# therefore use a distinct costume resource while the front actor stays P1.
assert read(0x490EDA,1) == b"\xe8"
assert 0x490EDF+struct.unpack("<i",read(0x490EDB,4))[0] == 0x4CAC20
print("PASS supported EXE: all menu relocation, loop bounds, calls and navigation stack operands")
