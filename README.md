# Spider-Man 2000 Co-op

A work-in-progress, two-player adaptation of [krystalgamer/spidey-decomp](https://github.com/krystalgamer/spidey-decomp), the PC Spider-Man decompilation. This repository is a GitHub fork and preserves the upstream history and attribution. The co-op changes are maintained by [Alin-Alexandru-Ion](https://github.com/Alin-Alexandru-Ion) with OpenAI Codex assistance.

The co-op implementation spans the original runtime files and [`coop/`](coop/README.md). The `COOP_*.txt` files document the camera, HUD, input, world, and scripted-event work. [`tests/coop/`](tests/coop/) contains focused regression checks. These checks do not establish that every level and boss is fully playable; live game testing remains necessary.

## Building

The legacy PC build entry point is `build.bat`. It expects the Visual C++ 6 toolchain under `C:\vs` and invokes `nmake` with `spider.mak`. Use your own legally obtained Spider-Man PC installation and required runtime dependencies. This repository does not provide a ready-to-play game release.

## Rights and attribution

The upstream repository does not declare a repository-wide license in the source checked for this fork. This fork does not claim ownership of the upstream source, its bundled third-party files, or the original game's intellectual property, and it does not grant a license for them. Co-op contributions are identified by the fork's commits and documentation. Check the rights for each component before redistributing it outside GitHub's fork functionality.
