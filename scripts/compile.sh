#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    echo "Usage: $0 <program.wlp4>" >&2
    exit 1
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SOURCE_FILE="$(realpath "$1")"

if [[ ! -f "$SOURCE_FILE" ]]; then
    echo "Input file not found: $1" >&2
    exit 1
fi

make -C "$ROOT_DIR" >/dev/null

(
    cd "$ROOT_DIR/src/scanner"
    "$ROOT_DIR/build/wlp4scan" < "$SOURCE_FILE"
) | "$ROOT_DIR/build/wlp4parse" \
  | "$ROOT_DIR/build/wlp4type" \
  | "$ROOT_DIR/build/wlp4gen"

