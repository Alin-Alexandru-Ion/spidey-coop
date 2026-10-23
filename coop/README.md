# Co-op source layout

The co-op implementation is header-only because the original VS6 project keeps
the executable's patched hook definitions in `main.cpp` and `PCTex.cpp`. Those
original translation units remain at the repository root. The headers are grouped
by ownership, and cross-category includes are relative to `coop/` so each
dependency is visible at the use site.

| Directory | Responsibility |
| --- | --- |
| `core` | player registry, routing, scoped retail-global compatibility banks, session carry |
| `player` | CPlayer lifecycle, spawn, costume, health and armor |
| `input` | bindings and controller routing |
| `world` | collision and section activation around both players |
| `camera` | camera policy, aim and view geometry |
| `presentation` | HUD, markers, effects, display, texture remakes and diagnostics |
| `script` | cinematic, authored motion and control handoffs |
| `interaction` | enemy, boss, projectile and object adapters |
| `menu` | co-op settings and menu preview |

Native absolute addresses in these headers refer to the supported retail build.
The comments on hook sites explain the callback contract and why state must be
restored. Do not reuse addresses for a different EXE without identifying and
testing its corresponding code. `tests/coop/run_router_tests.py` compiles the
production headers against small fake retail objects; validation scripts check
source hook placement. `build.bat` uses the VS6 `spider.mak` project.
