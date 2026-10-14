"""Keep noisy SDCC _Optional warnings visible while enforcing other warnings."""
import re
import subprocess
import sys


NON_FATAL = {355, 358, 359}


def fatal_warnings(stderr):
    unexpected = []
    for line in stderr.splitlines():
        match = re.search(r"\bwarning (\d+):", line)
        if match:
            if int(match.group(1)) not in NON_FATAL:
                unexpected.append(line)
        elif re.search(r"\bwarning:", line):
            unexpected.append(line)
    return unexpected


def main():
    # Replace the compiler's blanket --Werror with the policy above.
    # Print every diagnostic and preserve compiler errors unchanged.
    command = [arg for arg in sys.argv[1:] if arg != "--Werror"]
    result = subprocess.run(command, capture_output=True, text=True)
    sys.stdout.write(result.stdout)
    sys.stderr.write(result.stderr)
    if result.returncode:
        return result.returncode
    fatal = fatal_warnings(result.stderr)
    if fatal:
        sys.stderr.write("SDCC emitted fatal warnings:\n" + "\n".join(fatal) + "\n")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
