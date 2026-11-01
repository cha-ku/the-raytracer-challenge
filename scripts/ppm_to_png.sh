#!/usr/bin/env bash
# Convert a PPM render to PNG using ImageMagick.
#
# Usage:
#   scripts/ppm_to_png.sh <input.ppm> [output.png]
#
# If output.png is omitted, the PNG is written next to the input with the same name.

set -euo pipefail

if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "Usage: $0 <input.ppm> [output.png]" >&2
    exit 1
fi

input="$1"
output="${2:-${input%.ppm}.png}"

if [[ ! -f "$input" ]]; then
    echo "Error: '$input' not found" >&2
    exit 1
fi

if ! command -v magick >/dev/null 2>&1; then
    echo "Error: ImageMagick ('magick') is not installed" >&2
    exit 1
fi

magick "$input" "$output"
echo "Wrote $output"
