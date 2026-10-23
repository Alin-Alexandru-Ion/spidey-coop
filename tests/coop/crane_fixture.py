"""Use the installed crane script bytes for dispatch and transition regressions."""
import struct
import zlib

def generate(repo, work):
    data = (repo.parent / "Spider-Man-decomp-baseline/data.pkr").read_bytes()
    directory = struct.unpack_from("<I", data, 4)[0]
    _, directories, files = struct.unpack_from("<III", data, directory)
    directory += 12 + directories * 40
    for i in range(files):
        name, _, compression, position, _, packed = struct.unpack_from("<32sIiIII", data, directory + i * 52)
        if name.split(b"\0", 1)[0].upper() == b"L3A4_T.TRG":
            raw = data[position:position + packed]
            raw = zlib.decompress(raw) if compression == 2 else raw
            break
    else:
        raise AssertionError("Installed crane trigger bank missing")
    count = struct.unpack_from("<I", raw, 8)[0]
    offsets = struct.unpack_from("<" + "I" * count, raw, 12)
    assert count > 159 and len(raw) % 2 == 0
    commands = {}
    for node in (156, 158):
        assert struct.unpack_from("<HH", raw, offsets[node]) == (6, 0)
        commands[node] = (((offsets[node] + 4 + 3) & ~3) + 4) // 2
    words = struct.unpack("<" + "H" * (len(raw) // 2), raw)
    (work / "crane_fixture.h").write_text(
        "static unsigned short craneBankWords[]={" + ",".join(map(str, words)) + "};\n"
        "static unsigned craneNodeOffsets[]={" + ",".join(str(x // 2) for x in offsets) + "};\n"
        "static const unsigned craneFixtureCount=" + str(count) + ";\n"
        "static const unsigned craneRouteCommandWord=" + str(commands[158]) + ";\n"
        "static const unsigned craneExitCommandWord=" + str(commands[156]) + ";\n")
    for i in range(files):
        name, _, compression, position, _, packed = struct.unpack_from("<32sIiIII", data, directory + i * 52)
        if name.split(b"\0", 1)[0].upper() == b"L3A5_T.TRG":
            raw = data[position:position + packed]
            raw = zlib.decompress(raw) if compression == 2 else raw
            break
    else:
        raise AssertionError("Installed post-crane trigger bank missing")
    count = struct.unpack_from("<I", raw, 8)[0]
    offsets = struct.unpack_from("<" + "I" * count, raw, 12)
    assert struct.unpack_from("<HH", raw, offsets[22]) == (6, 0)
    assert struct.unpack_from("<3H", raw, offsets[26]) == (3, 1, 242)
    assert struct.unpack_from("<H", raw, offsets[242])[0] == 3
    command = (((offsets[22] + 4 + 3) & ~3) + 4) // 2
    words = struct.unpack("<" + "H" * (len(raw) // 2), raw)
    with (work / "crane_fixture.h").open("a") as fixture:
        fixture.write(
            "static unsigned short arrivalBankWords[]={" + ",".join(map(str, words)) + "};\n"
            "static unsigned arrivalNodeOffsets[]={" + ",".join(str(x // 2) for x in offsets) + "};\n"
            "static const unsigned arrivalFixtureCount=" + str(count) + ";\n"
            "static const unsigned arrivalCommandWord=" + str(command) + ";\n")
