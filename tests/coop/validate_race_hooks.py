"""Verify the native chase/race-progress modules in the supported retail EXE."""
from pathlib import Path
from source_paths import header_path
import hashlib
import struct

repo = Path(__file__).resolve().parents[2]
image = (repo.parent / "Spider-Man-decomp-baseline" / "SpideyPC.exe").read_bytes()
assert hashlib.sha256(image).hexdigest() == "d55a0bb0e920c497ce1ca76f08ed2e62feefcb6ff3c2901c0d59890f099ba93c"
pe = struct.unpack_from("<I", image, 60)[0]
optional_size = struct.unpack_from("<H", image, pe + 20)[0]
sections = [struct.unpack_from("<IIII", image, pe + 24 + optional_size + i*40 + 8)
            for i in range(struct.unpack_from("<H", image, pe + 6)[0])]

def read(address, size):
    rva = address - 0x400000
    for virtual_size, virtual_address, raw_size, raw_address in sections:
        if virtual_address <= rva < virtual_address + max(virtual_size, raw_size):
            offset = raw_address + rva - virtual_address
            return image[offset:offset + size]
    raise AssertionError(hex(address))

for name, name_address, pointer_address, native_init, native_progress in (
    (b"l2a1lsc", 0x551B94, 0x551BA4, 0x447640, 0x4477F0),
    (b"venom",   0x551C34, 0x551C44, 0x4E7D30, 0x4E7E10),
    (b"l5a5lsc", 0x551C70, 0x551C80, 0x448730, 0x4489B0),
    (b"superock", 0x551D88, 0x551D98, 0x4CBD70, 0x4D0E70),
):
    assert read(name_address, len(name)) == name
    assert struct.unpack("<I", read(pointer_address, 4))[0] == native_init
    assert read(native_init, 8) != b"\0" * 8
    assert read(native_progress, 8) != b"\0" * 8

source = (header_path(repo, "coop_interaction_runtime.h")).read_text()
for site in ("0x00551BA4", "0x00551C44", "0x00551C80", "0x00551D98"):
    assert site in source
print("PASS supported EXE: four race/chase module entries and native progress callbacks")
