#!/usr/bin/python

import argparse
import sys
import os

parser = argparse.ArgumentParser(description='Convert 8x8 font hex file to binary')
parser.add_argument('-f', '--file', required=True, help='Input font file in the Unifont .hex format')
parser.add_argument('-c', '--codepage', required=True, help='Text file with 256 Unicode codepoints (in hex) to include in the font')
parser.add_argument('-o', '--output', required=True, help='Output .bin file')

args = parser.parse_args()

# Load the codepoint list
codepoints = []
try:
    with open(args.codepage, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith('#'):
                try:
                    cp = int(line, 16)
                    if 0 <= cp <= 0x10FFFF:
                        codepoints.append(cp)
                    else:
                        print(f"Warning: Codepoint {line} out of range, skipping")
                except ValueError:
                    print(f"Warning: Invalid codepoint '{line}', skipping")
except FileNotFoundError:
    print(f"Error: Codepage file '{args.codepage}' not found")
    sys.exit(1)
if len(codepoints) != 256:
    print(f"Error: Codepage file must contain exactly 256 valid codepoints, found {len(codepoints)}")
    sys.exit(1)

# Load the font hex file
bitmaps = {}
try:
    with open(args.file, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith('#'):
                parts = line.split(':')
                if len(parts) != 2:
                    continue
                try:
                    cp = int(parts[0], 16)
                    bitmap_hex = parts[1]
                    if len(bitmap_hex) != 16:
                        continue
                    bitmap = bytes(int(bitmap_hex[i:i+2], 16) for i in range(0, 16, 2))
                    bitmaps[cp] = bitmap
                except ValueError:
                    continue
except FileNotFoundError:
    print(f"Error: Font file '{args.file}' not found")
    sys.exit(1)

# Create the output binary font
font_data = bytearray()
for cp in codepoints:
    if cp in bitmaps:
        font_data.extend(bitmaps[cp])
    else:
        font_data.extend(b'\x00' * 8)  # Empty bitmap for missing codepoints
        print(f"Warning: Codepoint U+{cp:04X} for index {codepoints.index(cp)} not found in font, using empty bitmap")

# Write the output binary file
try:
    with open(args.output, 'wb') as f:
        f.write(font_data)
    print(f"Wrote {len(font_data)} bytes to '{args.output}'")
except IOError as e:
    print(f"Error: Could not write to output file '{args.output}': {e}")
    sys.exit(1)
