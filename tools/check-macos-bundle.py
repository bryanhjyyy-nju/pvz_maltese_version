"""Check deployment dependencies and launch the app without an installed Qt SDK."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile

app = Path(sys.argv[1]).resolve()
executable = app / "Contents/MacOS/PvZ_demo"
if sys.platform != "darwin" or not executable.is_file():
    raise SystemExit("Expected a macOS PvZ_demo.app bundle")

for path in (app / "Contents").rglob("*"):
    if not path.is_file() or path.is_symlink():
        continue
    kind = subprocess.check_output(["file", "-b", str(path)], text=True)
    if "Mach-O" not in kind:
        continue
    subprocess.run(["lipo", "-verify_arch", "arm64", "x86_64", str(path)], check=True)
    for architecture in ("arm64", "x86_64"):
        links = subprocess.check_output(["otool", "-arch", architecture, "-L", str(path)], text=True)
        for line in links.splitlines()[1:]:
            dependency = line.strip().split(" (", 1)[0]
            if dependency.startswith("/") and not dependency.startswith(("/System/Library/", "/usr/lib/")):
                raise SystemExit(f"External dependency in {path}: {dependency}")

environment = {key: value for key, value in os.environ.items()
               if not key.startswith(("QT_", "QML_", "DYLD_"))}
environment["PATH"] = "/usr/bin:/bin:/usr/sbin:/sbin"
# Keep the smoke launch's settings and saves away from the player's profile.
with tempfile.TemporaryDirectory(prefix="pvz-smoke-") as directory:
    environment["HOME"] = directory
    environment["CFFIXED_USER_HOME"] = directory
    with open(Path(directory) / "launch.log", "w+") as log:
        process = subprocess.Popen([str(executable)], cwd=directory, env=environment,
                                   stdout=log, stderr=subprocess.STDOUT)
        try:
            process.wait(timeout=8)
            log.seek(0)
            raise SystemExit(f"App exited before startup check ({process.returncode}):\n{log.read()}")
        except subprocess.TimeoutExpired:
            print(f"Bundle started successfully on {os.uname().machine}, with no external Qt environment")
        finally:
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
