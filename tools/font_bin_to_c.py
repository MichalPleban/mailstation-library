#!/usr/bin/python

import argparse
import sys
import os

parser = argparse.ArgumentParser(description='Convert 8x8 font binary file to C array')
parser.add_argument('-f', '--file', required=True, help='Input font file .bin format')
parser.add_argument('-o', '--output', required=True, help='Output C source file')

args = parser.parse_args()

# Load the font binary file
try:
    with open(args.file, 'rb') as f:
        font_data = f.read()
except FileNotFoundError:
    print(f"Error: Font file '{args.file}' not found")
    sys.exit(1)

if len(font_data) != 2048:
    print(f"Error: Font file '{args.file}' must be exactly 2048 bytes, found {len(font_data)} bytes")
    sys.exit(1)

# Create the output C source file
try:
    with open(args.output, 'w', encoding='utf-8') as f:
        f.write('#include <stdint.h>\n\n')
        f.write('#include \"mailstation.h\"\n\n')
        f.write('const uint8_t ms_screen_font[] = {\n')
        for i in range(256):
            f.write('    ' + ', '.join(f'0x{font_data[i*8 + j]:02X}' for j in range(8)))
            if i < 255:
                f.write(',')
            f.write(f'\n')
        f.write('};\n')
except Exception as e:
    print(f"Error: Could not write to output file '{args.output}': {e}")
    sys.exit(1)
