"""Exercise only the retail projection arithmetic, not the game entrypoint.
Requires the exact supported EXE. No game process, renderer, device or imports run.
The temporary harness relocates selected arithmetic operands in its private copy.
"""
from pathlib import Path
from source_paths import header_path
import hashlib
import os
import subprocess
import sys
import tempfile
repo = Path(__file__).resolve().parents[2]
exe = Path(sys.argv[1]) if len(sys.argv)>1 else repo.parent/"Spider-Man-decomp-baseline"/"SpideyPC.exe"
EXPECTED = "d55a0bb0e920c497ce1ca76f08ed2e62feefcb6ff3c2901c0d59890f099ba93c"
if hashlib.sha256(exe.read_bytes()).hexdigest()!=EXPECTED:
    raise SystemExit("Unsupported EXE: refusing to execute arithmetic test")
env = os.environ.copy()
vs = Path(env.get("MSVCDir", r"C:\vs"))
env["INCLUDE"] = str(vs/"INCLUDE")
env["LIB"] = str(vs/"LIB")
with tempfile.TemporaryDirectory(prefix="spidey-frustum-test-") as folder:
    work = Path(folder)
    (work/"frustum_test.cpp").write_bytes(Path(__file__).with_name("frustum_test.cpp").read_bytes())
    (work / "coop/camera/coop_view.h").parent.mkdir(parents=True, exist_ok=True)
    (work / "coop/camera/coop_view.h").write_bytes(header_path(repo, "coop_view.h").read_bytes())
    (work/"retail_frustum_helpers.h").write_bytes(Path(__file__).with_name("retail_frustum_helpers.h").read_bytes())
    subprocess.run([str(vs/"BIN"/"cl.exe"),"/nologo","/I",".","frustum_test.cpp",
                    "/link","/BASE:0x10000000","/FIXED","kernel32.lib"],cwd=work,env=env,check=True)
    subprocess.run([str(work/"frustum_test.exe"),str(exe)],cwd=work,check=True,timeout=15)
