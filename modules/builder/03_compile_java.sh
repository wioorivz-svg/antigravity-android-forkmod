#!/usr/bin/env bash
# 03_compile_java.sh - Compiles Java sources and generated classes.
set -euo pipefail

OBJ_DIR="${1:-$BUILD_DIR/obj}"
GEN_DIR="${2:-$BUILD_DIR/gen}"
SRC_DIR="${3:-$APP_DIR/src/main/java}"
ANDROID_JAR="${4:-$TOOLS_DIR/android.jar}"

echo "[*] [03_compile_java] Compiling Java sources..."
mkdir -p "$OBJ_DIR"

BUILD_DIR="${BUILD_DIR:-$(dirname "$OBJ_DIR")}"
ARG_FILE="$BUILD_DIR/java_sources.txt"

JAVA_COUNT=$(find "$SRC_DIR" -name '*.java' | wc -l)
GEN_COUNT=$(find "$GEN_DIR" -name '*.java' | wc -l)

[ "$JAVA_COUNT" -gt 0 ] || { echo "Error: no java sources found under $SRC_DIR" >&2; exit 1; }
[ "$GEN_COUNT" -gt 0 ] || { echo "Error: no generated sources found under $GEN_DIR" >&2; exit 1; }

mkdir -p "$BUILD_DIR"
if command -v cygpath >/dev/null 2>&1; then
    find "$SRC_DIR" "$GEN_DIR" -name '*.java' | while read -r line; do
        echo "\"$(cygpath -m "$line")\""
    done > "$ARG_FILE"
    OBJ_DIR="$(cygpath -m "$OBJ_DIR")"
    ANDROID_JAR="$(cygpath -m "$ANDROID_JAR")"
    ARG_FILE_WIN="$(cygpath -m "$ARG_FILE")"
else
    find "$SRC_DIR" "$GEN_DIR" -name '*.java' | sed 's/.*/"&"/' > "$ARG_FILE"
    ARG_FILE_WIN="$ARG_FILE"
fi

javac -encoding UTF-8 -J-Dfile.encoding=UTF-8 \
    -source 1.8 -target 1.8 \
    -d "$OBJ_DIR" \
    -cp "$ANDROID_JAR" \
    "@$ARG_FILE_WIN"

echo "[OK] [03_compile_java] Compiled $(find "$OBJ_DIR" -name '*.class' | wc -l) classes into $OBJ_DIR"
