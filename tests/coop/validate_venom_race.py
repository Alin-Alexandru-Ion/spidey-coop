"""Guard the co-op race failure adapter against the installed trigger bank."""
from pathlib import Path
from source_paths import header_path
import struct
import zlib

repo = Path(__file__).resolve().parents[2]
archive = (repo.parent / "Spider-Man-decomp-baseline" / "data.pkr").read_bytes()
directory = struct.unpack_from("<I", archive, 4)[0]
_, directory_count, file_count = struct.unpack_from("<III", archive, directory)
directory += 12 + directory_count * 40
for i in range(file_count):
    name, _, compression, position, _, packed = struct.unpack_from(
        "<32sIiIII", archive, directory + i * 52
    )
    if name.split(b"\0", 1)[0].upper() == b"L5A1_T.TRG":
        bank = archive[position:position + packed]
        bank = zlib.decompress(bank) if compression == 2 else bank
        break
else:
    raise AssertionError("L5A1 trigger bank missing")

count = struct.unpack_from("<I", bank, 8)[0]
offsets = struct.unpack_from("<" + "I" * count, bank, 12)
assert count > 330
start, end = offsets[316:318]
assert struct.unpack_from("<HH", bank, start) == (6, 0)
command = ((start + 4 + 3) & ~3) + 4
assert struct.unpack_from("<H", bank, command)[0] == 218
assert bank[command + 6:command + 21] == b"You Lost Venom\0"
start = offsets[330]
assert struct.unpack_from("<H", bank, start)[0] == 1
links = struct.unpack_from("<H", bank, start + 6)[0]
assert 316 in struct.unpack_from("<" + "H" * links, bank, start + 8)
assert b"Inside\0" in bank and b"Outside\0" in bank
def command_bytes(node):
    start, end = offsets[node:node + 2]
    assert struct.unpack_from("<H", bank, start)[0] == 6
    return bank[start + 8:end]

# The window path uses native synthesized input for whichever CPlayer owns
# its collision pulse. Its earlier visibility pulse contains an unrelated
# TRGP_Wait command before the Inside release, so projection must scan both.
assert command_bytes(45).startswith(b"\xbf\0TRGP_Wait\0")
assert b"\xbf\0Inside\0\0\1\0\1\0\0\0" in command_bytes(45)
for node in (304, 311):
    assert struct.unpack_from("<HHHHHH", command_bytes(node)) == (195, 30, 199, 0, 1, 310)
assert b"\xbf\0Inside\0\0\1\0\1\0\1\0" in command_bytes(338)
source = (repo / "main.cpp").read_text()
assert "CoOp_DeferTrailingRaceFailure(commands,node,owner)" in source
assert "CoOp_DeferRaceContact(point->NodeIndex,owner)" in source
assert 'strcmp(reinterpret_cast<const char*>(commands+3),"You Lost Venom")' in source
interaction = (header_path(repo, "coop_interaction_runtime.h")).read_text()
exe = (repo.parent / "Spider-Man-decomp-baseline" / "SpideyPC.exe").read_bytes()
pe = struct.unpack_from("<I", exe, 0x3C)[0]
base = struct.unpack_from("<I", exe, pe + 24 + 28)[0]
sections = struct.unpack_from("<H", exe, pe + 6)[0]
section_table = pe + 24 + struct.unpack_from("<H", exe, pe + 20)[0]
for i in range(sections):
    sec = section_table + 40 * i
    size, va, raw_size, raw = struct.unpack_from("<IIII", exe, sec + 8)
    if va <= 0x00551C44 - base < va + min(size, raw_size):
        assert struct.unpack_from("<I", exe, raw + 0x00551C44 - base - va)[0] == 0x004E7D30
        break
else:
    raise AssertionError("Venom relocatable module table entry missing")
assert "module->field_C[1]=CoOp_VenomProgress" in interaction
assert "CoOp_InstallRaceProgressHooks()" in source
assert "CoOp_VenomRelocInit" in interaction
assert "CoOp_IsProtectedSection(prefix,gCoOpSectionProfile)" in source
assert 'CoOp_SectionVisibility("Inside",1,1,true)' in source
print("PASS Venom race failure, visibility and synthesized window path match installed bank")
