"""Locate a moved co-op header in the source tree or the temporary test tree."""
from pathlib import Path

def header_path(root: Path, name: str) -> Path:
    matches = tuple((root / "coop").rglob(name))
    if len(matches) != 1:
        raise FileNotFoundError(f"Expected one co-op header {name}, found {len(matches)}")
    return matches[0]
