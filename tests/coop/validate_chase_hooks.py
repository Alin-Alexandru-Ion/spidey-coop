"""Verify chase vtable guards against the actual supported retail executable."""
from pathlib import Path
from source_paths import header_path
import re
import struct

repo = Path(__file__).resolve().parents[2]
exe = (repo.parent / "Spider-Man-decomp-baseline" / "SpideyPC.exe").read_bytes()
source = (header_path(repo, "coop_interaction_runtime.h")).read_text()

pe = struct.unpack_from("<I", exe, 0x3C)[0]
assert exe[pe:pe + 4] == b"PE\0\0"
sections = struct.unpack_from("<H", exe, pe + 6)[0]
optional = pe + 24
assert struct.unpack_from("<H", exe, optional)[0] == 0x10B
image_base = struct.unpack_from("<I", exe, optional + 28)[0]
section_table = optional + struct.unpack_from("<H", exe, pe + 20)[0]

def read_va(address):
    rva = address - image_base
    for i in range(sections):
        sec = section_table + i * 40
        virtual_size, virtual_address, raw_size, raw_address = struct.unpack_from("<IIII", exe, sec + 8)
        if virtual_address <= rva < virtual_address + min(virtual_size, raw_size):
            return struct.unpack_from("<I", exe, raw_address + rva - virtual_address)[0]
    raise AssertionError(f"VA {address:08X} is not in a mapped section")

entries = {
    "Scorpion": (0x0053BE34, 0x00488590, "CoOp_ScorpionAI"),
    "Rhino": (0x0053BDC4, 0x004819C0, "CoOp_RhinoAI"),
    "Venom": (0x0053C6C0, 0x004EC040, "CoOp_VenomAI"),
    "Cop": (0x0053B678, 0x0042FA10, "CoOp_CopAI"),
    "Thug": (0x0053C558, 0x004DB280, "CoOp_ThugAI"),
    "Chopper": (0x0053B5F8, 0x004262A0, "CoOp_ChopperAI"),
    "Sniper": (0x0053B5A0, 0x00421900, "CoOp_SniperAI"),
    "Searchlight": (0x0053B5C0, 0x00422EF0, "CoOp_SearchlightAI"),
    "Missile": (0x0053B5E0, 0x00425400, "CoOp_MissileAI"),
    "Machine-gun bullet": (0x0053B594, 0x00421100, "CoOp_BulletMove"),
}
for name, (site, original, wrapper) in entries.items():
    assert read_va(site) == original, (name, hex(site), hex(read_va(site)))
    guard = rf"\*reinterpret_cast<u32\*>\(0x{site:08X}\)!=0x{original:08X}"
    patch = rf"\*reinterpret_cast<u32\*>\(0x{site:08X}\)=reinterpret_cast<u32>\(&{wrapper}\)"
    assert re.search(guard, source), f"{name} executable guard drifted"
    assert re.search(patch, source), f"{name} patch target drifted"
camera_call = 0x004555FC
def read_byte(address):
    rva = address - image_base
    for i in range(sections):
        sec = section_table + i * 40
        virtual_size, virtual_address, raw_size, raw_address = struct.unpack_from("<IIII", exe, sec + 8)
        if virtual_address <= rva < virtual_address + min(virtual_size, raw_size):
            return raw_address + rva - virtual_address
    raise AssertionError(f"VA {address:08X} is not in a mapped section")
raw = read_byte(camera_call)
assert exe[raw] == 0xE8
assert camera_call + 5 + struct.unpack_from("<i", exe, raw + 1)[0] == 0x00460FC0
assert re.search(r"static bool CoOp_InstallCameraUpdateHook\(\)", source)
assert re.search(r"PATCH_CALL\(0x004555FC,CoOp_UpdateCameras\)", source)
interactions = source.split("static bool CoOp_InstallInteractionHooks()", 1)[1]
assert "CoOp_UpdateCameras" not in interactions, "camera tick depends on unrelated interaction guards"
for address, expected in {
    0x004557D6: bytes.fromhex("663d4201"),
    0x004557E0: bytes.fromhex("ff5014"),
    0x004557E5: bytes.fromhex("663d4301"),
    0x004557EF: bytes.fromhex("ff5214"),
    0x0045581B: bytes.fromhex("663d4101"),
    0x00455825: bytes.fromhex("ff5014"),
}.items():
    assert exe[read_byte(address):read_byte(address) + len(expected)] == expected
render = (repo / "main.cpp").read_text()
assert "CoOp_DrawChaseSpecialList(0x0056E994,322,323)" in render
assert "CoOp_DrawChaseSpecialList(0x0056E990,321,321)" in render
print("PASS retail AI entries and independent camera call: executable bytes, guards and wrappers agree")
