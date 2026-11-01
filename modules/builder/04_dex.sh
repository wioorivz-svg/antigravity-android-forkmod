#!/usr/bin/env bash
# 04_dex.sh - Converts compiled Java bytecode into classes.dex via D8/R8.
set -euo pipefail

DEX_DIR="${1:-$BUILD_DIR/dex}"
OBJ_DIR="${2:-$BUILD_DIR/obj}"
R8_JAR="${3:-$TOOLS_DIR/r8.jar}"
ANDROID_JAR="${4:-$TOOLS_DIR/android.jar}"
MIN_API="${5:-24}"

BUILD_DIR="${BUILD_DIR:-$(dirname "$OBJ_DIR")}"
CLASS_LIST="$BUILD_DIR/class_files.txt"

echo "[*] [04_dex] Running D8 dexer..."
mkdir -p "$DEX_DIR" "$BUILD_DIR"

if command -v cygpath >/dev/null 2>&1; then
    find "$OBJ_DIR" -name '*.class' | while read -r line; do
        cygpath -m "$line"
    done > "$CLASS_LIST"
    DEX_DIR_WIN="$(cygpath -m "$DEX_DIR")"
    R8_JAR_WIN="$(cygpath -m "$R8_JAR")"
    ANDROID_JAR_WIN="$(cygpath -m "$ANDROID_JAR")"
    CLASS_LIST_WIN="$(cygpath -m "$CLASS_LIST")"
else
    find "$OBJ_DIR" -name '*.class' > "$CLASS_LIST"
    DEX_DIR_WIN="$DEX_DIR"
    R8_JAR_WIN="$R8_JAR"
    ANDROID_JAR_WIN="$ANDROID_JAR"
    CLASS_LIST_WIN="$CLASS_LIST"
fi
[ -s "$CLASS_LIST" ] || { echo "Error: no class files found under $OBJ_DIR" >&2; exit 1; }

# D8 CLI does not accept raw directories directly (throws 'Unsupported source file type').
# We pass classes via @argfile to eliminate ARG_MAX command line overflow.
java -cp "$R8_JAR_WIN" com.android.tools.r8.D8 \
    --output "$DEX_DIR_WIN" \
    --lib "$ANDROID_JAR_WIN" \
    --min-api "$MIN_API" \
    "@$CLASS_LIST_WIN"

[ -f "$DEX_DIR/classes.dex" ] || { echo "Error: D8 did not produce classes.dex" >&2; exit 1; }
echo "[OK] [04_dex] Produced $(stat -c%s "$DEX_DIR/classes.dex") bytes classes.dex"
