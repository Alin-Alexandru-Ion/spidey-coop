"""Guard the retail three-argument controller poll site used by the P2 XInput hook."""
from pathlib import Path
import struct

exe = Path(__file__).resolve().parents[2].parent / "Spider-Man-decomp-baseline" / "SpideyPC.exe"
data = exe.read_bytes()
pe = struct.unpack_from("<I", data, 0x3C)[0]
sections = struct.unpack_from("<H", data, pe + 6)[0]
optional_size = struct.unpack_from("<H", data, pe + 20)[0]
base = struct.unpack_from("<I", data, pe + 24 + 28)[0]
site = 0x0050A2AD
offset = None
for index in range(sections):
    header = pe + 24 + optional_size + index * 40
    virtual_size, rva, _, raw = struct.unpack_from("<IIII", data, header + 8)
    if rva <= site - base < rva + virtual_size:
        offset = raw + site - base - rva
        break
assert offset is not None
assert data[offset - 15:offset] == bytes.fromhex(
    "68 94 83 56 00 68 14 0A AC 00 68 10 0A AC 00")
assert data[offset] == 0xE8
target = site + 5 + struct.unpack_from("<i", data, offset + 1)[0]
assert target == 0x00501E50
print("PASS retail controller poll: three pointer arguments and expected call target")
