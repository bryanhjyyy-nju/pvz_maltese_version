#!/usr/bin/env bash
set -euo pipefail

if [[ "$(uname -s)" != Darwin ]]; then
    echo 'This script needs macOS, Xcode command line tools, and Qt 6.8 with Multimedia.' >&2
    exit 1
fi

run_tests=false
case "${1:-}" in
    '') ;;
    --test) run_tests=true ;;
    *) echo "Usage: bash tools/build-macos.sh [--test]" >&2; exit 1 ;;
esac

project_root="$(cd "$(dirname "$0")/.." && pwd)"
build_dir="$project_root/build-macos"
qt_bin="${QT_ROOT_DIR:+$QT_ROOT_DIR/bin}"
if [[ -n "$qt_bin" ]]; then export PATH="$qt_bin:$PATH"; fi
for tool in qmake macdeployqt make python3; do command -v "$tool" >/dev/null; done
qmake -v
jobs="${PVZ_BUILD_JOBS:-2}"
mkdir -p "$build_dir/game" "$build_dir/dist"
cd "$build_dir/game"
# Qt's official macOS packages include both architectures.
qmake "$project_root/PvZ_demo.pro" CONFIG+=release CONFIG-=debug 'QMAKE_APPLE_DEVICE_ARCHS=arm64 x86_64'
make -j"$jobs"

if $run_tests; then
    mkdir -p "$build_dir/tests/screenshots"
    cd "$build_dir/tests"
    qmake "$project_root/pvz_tests.pro" CONFIG+=release CONFIG-=debug
    make -j"$jobs"
    PVZ_CAPTURE_DIR="$build_dir/tests/screenshots" ./pvz_tests -platform offscreen -o test-results.txt,txt
    PVZ_CAPTURE_DIR="$build_dir/tests/native-screenshots" ./pvz_tests -platform cocoa \
        audioAssetsLoad renderScreens battlefieldPauseAndPlacement continueFlow \
        -o native-test-results.txt,txt
fi

cd "$build_dir"
app="$build_dir/dist/PvZ_demo.app"
# ditto replaces the bundle files without losing their executable permissions.
if [[ -e "$app" ]]; then
    echo "Output already exists: $app. Use a fresh build-macos/dist directory." >&2
    exit 1
fi
ditto "$build_dir/game/PvZ_demo.app" "$app"
macdeployqt "$app" -verbose=2 -codesign=-
codesign --verify --deep --strict "$app"
lipo -verify_arch arm64 x86_64 "$app/Contents/MacOS/PvZ_demo"
python3 "$project_root/tools/check-macos-bundle.py" "$app"
cp "$project_root/packaging/macos/使用说明.txt" "$build_dir/dist/使用说明.txt"
ditto -c -k --sequesterRsrc --keepParent "$app" "$build_dir/dist/PvZ_demo-macOS-universal.zip"
cd "$build_dir/dist"
shasum -a 256 PvZ_demo-macOS-universal.zip > SHA256SUMS.txt
echo "Mac application: $app"
echo "Mac download: $build_dir/dist/PvZ_demo-macOS-universal.zip"
