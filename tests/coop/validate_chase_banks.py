"""Audit the installed helicopter chase trigger banks through L4A1."""
from pathlib import Path
import struct
import zlib

data = (Path(__file__).resolve().parents[3] / "Spider-Man-decomp-baseline" / "data.pkr").read_bytes()
directory = struct.unpack_from("<I", data, 4)[0]
_, directories, files = struct.unpack_from("<III", data, directory)
directory += 12 + directories * 40
assets = {}
for i in range(files):
    name, _, compression, position, _, packed = struct.unpack_from("<32sIiIII", data, directory + i * 52)
    name = name.split(b"\0", 1)[0].decode("ascii", "ignore").upper()
    if name in {"L3A1_T.TRG", "L3A1A_T.TRG", "L3A2_T.TRG", "L3A3_T.TRG", "L3A4_T.TRG", "L3A5_T.TRG"}:
        payload = data[position:position + packed]
        assets[name] = zlib.decompress(payload) if compression == 2 else payload

chain = {
    "L3A1_T.TRG": {"L3A1a_T": 278, "L3A2_T": 36},
    "L3A1A_T.TRG": {"L3A2_T": 47},
    "L3A2_T.TRG": {"L3A3_T": 373},
    "L3A3_T.TRG": {"L3A4_T": 102},
    "L3A4_T.TRG": {"L3A5_T": 156},
    "L3A5_T.TRG": {"L4A1_T": 209},
}
camera_nodes = {
    "L3A1_T.TRG": {63: 186, 67: 186, 87: 186, 95: 186},
    "L3A1A_T.TRG": {51: 186, 55: 186, 68: 185, 69: 185},
    "L3A2_T.TRG": {7: 186, 11: 186, 437: 186, 438: 186},
    "L3A3_T.TRG": {28: 186, 32: 186},
    "L3A4_T.TRG": {7: 186, 11: 186, 74: 186, 88: 186},
    "L3A5_T.TRG": {6: 186, 10: 186, 17: 186, 19: 186},
}
assert set(assets) == set(chain)
for name, payload in assets.items():
    count = struct.unpack_from("<I", payload, 8)[0]
    offsets = struct.unpack_from("<" + "I" * count, payload, 12)
    assert any(struct.unpack_from("<H", payload, start)[0] == 4 for start in offsets), name + " has no autoexec"
    assert any(struct.unpack_from("<H", payload, start)[0] == 8 for start in offsets), name + " has no restart"
    for node, (opcode, target) in {
        **{number: (305, destination) for destination, number in chain[name].items()},
        **{number: (op, None) for number, op in camera_nodes[name].items()},
    }.items():
        start = offsets[node]
        assert struct.unpack_from("<H", payload, start)[0] == 6, (name, node)
        links = struct.unpack_from("<H", payload, start + 2)[0]
        command = ((start + 4 + links * 2 + 3) & ~3) + 4
        assert struct.unpack_from("<H", payload, command)[0] == opcode, (name, node)
        if target:
            assert target.encode() + b"\0" in payload[command:offsets[node + 1]], (name, node, target)
    if name == "L3A4_T.TRG":
        route = offsets[158]
        assert struct.unpack_from("<H", payload, route)[0] == 6
        route_command = ((route + 4 + 3) & ~3) + 4
        assert struct.unpack_from("<H", payload, route_command)[0] == 199
        crane = offsets[81]
        assert struct.unpack_from("<H", payload, crane)[0] == 6
        assert 158 in struct.unpack_from("<3H", payload, crane + 4)
        exit_node = offsets[156]
        assert struct.unpack_from("<H", payload, exit_node)[0] == 6
        exit_command = ((exit_node + 4 + 3) & ~3) + 4
        assert struct.unpack_from("<H", payload, exit_command)[0] == 305
        assert b"L3A5_T\0" in payload[exit_node:offsets[157]]
    if name == "L3A5_T.TRG":
        arrival = offsets[22]
        assert struct.unpack_from("<HH", payload, arrival) == (6, 0)
        command = ((arrival + 4 + 3) & ~3) + 4
        assert struct.unpack_from("<17H", payload, command) == (
            199, 0, 16, 26, 0, 2, 23, 72, 3, 7, 50, 0, 2, 26, 0, 255, 65535)
        # Native CutSceneSkipSnapPos uses 26 as the body destination and its
        # linked node 242 to restore facing/surface state at control handoff.
        assert struct.unpack_from("<HHH", payload, offsets[26]) == (3, 1, 242)
        assert struct.unpack_from("<H", payload, offsets[242])[0] == 3
    print(name, "nodes", count, "camera events", len(camera_nodes[name]), "exits", ", ".join(chain[name]))
print("PASS chase bank graph, camera directives, restart and autoexec through L4A1")
