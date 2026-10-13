"""Check a linked Z80 image and run it with uCsim for CTest."""
import re
import subprocess
import sys
from pathlib import Path


def main():
    simulator, image_name = sys.argv[1:]
    image = Path(image_name)
    output = image.with_name(image.stem + "-output.log")
    simulator_log = image.with_name(image.stem + "-simulator.log")
    for log in (output, simulator_log):
        log.unlink(missing_ok=True)

    # Keep code below the simulator interface and heap, and static data
    # below the stack. Reject a map we cannot inspect as well as an overlap.
    areas = set()
    for line in image.with_suffix(".map").read_text().splitlines():
        match = re.match(
            r"^(_CODE|_HOME|_GSINIT|_GSFINAL|_INITIALIZER|_DATA|_INITIALIZED)"
            r"\s+([0-9A-F]+)\s+([0-9A-F]+)", line)
        if match:
            area, address, size = match.groups()
            areas.add(area)
            limit = 0xf400 if area in ("_DATA", "_INITIALIZED") else 0xdf00
            if int(address, 16) + int(size, 16) > limit:
                raise SystemExit(f"{area} exceeds the test memory layout")
    if "_CODE" not in areas:
        raise SystemExit("No code region found in linker map")

    command = [simulator, "-t", "Z80", "-q", "-c", "-",
               "-I", f"if=rom[0xdf00],out={output}", str(image)]
    with simulator_log.open("w") as log:
        result = subprocess.run(command, input="run\nquit\n", text=True,
                                stdout=log, stderr=subprocess.STDOUT, timeout=60)
    transcript = output.read_text() if output.exists() else ""
    print(transcript, end="")
    if result.returncode or "CBUTIL TEST PASS" not in transcript.splitlines():
        raise SystemExit(f"Z80 test failed; see {simulator_log}")


if __name__ == "__main__":
    main()
