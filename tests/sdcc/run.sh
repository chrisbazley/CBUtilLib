#!/usr/bin/env bash
# Run the existing portable test groups on a simulated Z80.
set -euo pipefail
cd "$(dirname "$0")/../.."
build_dir="${CBUTIL_SDCC_BUILD_DIR:-build-sdcc}"
mkdir -p "$build_dir"
flags=(-mz80 --std-c99 --Werror --max-allocs-per-node 100 -DCBUTIL_FREESTANDING -DUSE_OPTIONAL -I.)
# 110/126 describe optimizer changes, including constant assertions.
# Trunk still cannot prove non-nullness for guarded members and assertions.
# Keep _Optional type checking, but exempt its inference warnings 355/358/359.
# Cap register-allocation search to keep these large test groups practical.
flags+=(--disable-warning 110 --disable-warning 126
        --disable-warning 355 --disable-warning 358 --disable-warning 359
        -Wp-include,tests/sdcc/compat.h)
sources=(LinkedList IntDict IntDictVIt StrDict StrDictVIt StringBuff StringBuf2
         StrInflate StrTail Stricmp Strnicmp)
objects=()
for source in "${sources[@]}"; do
  object="$build_dir/$source.rel"
  sdcc "${flags[@]}" -c "$source.c" -o "$object"
  objects+=("$object")
done
python3 - "$build_dir/CBUtil.lib" <<'PY_CLEAN'
import sys
from pathlib import Path
Path(sys.argv[1]).unlink(missing_ok=True)
PY_CLEAN
sdar -rc "$build_dir/CBUtil.lib" "${objects[@]}"
# Separate programs keep each test group small and identify failures.
groups=(List:LinkedList IntDic:intdict StrDic:strdict PtrDic:ptrdict
        StrBuf:StringBuffer StrExt:StrExtra Macro:MacroUtils)
if [[ "${1:-all}" != all ]]; then
  selected=()
  for group in "${groups[@]}"; do
    if [[ "${group%%:*}" == "$1" ]]; then
      selected+=("$group")
    fi
  done
  if [[ ${#selected[@]} == 0 ]]; then
    echo "Unknown test group: $1" >&2
    exit 1
  fi
  groups=("${selected[@]}")
fi
for group in "${groups[@]}"; do
  name="${group%%:*}"
  function="${group#*:}_tests"
  sdcc "${flags[@]}" -c "tests/${name}Test.c" -o "$build_dir/$name.rel"
  sdcc "${flags[@]}" -DTEST_FUNCTION="$function" -c tests/sdcc/runtime.c \
    -o "$build_dir/$name-runtime.rel"
  sdcc -mz80 --data-loc 0xf000 "$build_dir/$name-runtime.rel" \
    "$build_dir/$name.rel" "$build_dir/CBUtil.lib" -o "$build_dir/$name.ihx"
  # Reject images that overlap the fixed simulator, heap or stack regions.
  python3 - "$build_dir/$name.map" "$build_dir/$name-output.log" "$build_dir/$name-simulator.log" <<'PY_MEMORY'
import re
import sys
from pathlib import Path
for line in Path(sys.argv[1]).read_text().splitlines():
    match = re.match(r"^(_CODE|_HOME|_GSINIT|_GSFINAL|_INITIALIZER|_DATA|_INITIALIZED)\s+([0-9A-F]+)\s+([0-9A-F]+)", line)
    if match:
        area, address, size = match.groups()
        limit = 0xf400 if area in ("_DATA", "_INITIALIZED") else 0xdf00
        if int(address, 16) + int(size, 16) > limit:
            raise SystemExit(f"{area} exceeds the test memory layout")
for log in sys.argv[2:]:
    Path(log).unlink(missing_ok=True)
PY_MEMORY
  printf 'run\nquit\n' | timeout 60 ucsim_z80 -t Z80 -q -c - \
    -I "if=rom[0xdf00],out=$build_dir/$name-output.log" \
    "$build_dir/$name.ihx" > "$build_dir/$name-simulator.log" 2>&1
  cat "$build_dir/$name-output.log"
  grep -Fxq 'CBUTIL TEST PASS' "$build_dir/$name-output.log"
done
