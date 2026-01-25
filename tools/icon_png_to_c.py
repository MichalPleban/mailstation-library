#!/usr/bin/python

import argparse
import sys
import os
from PIL import Image

parser = argparse.ArgumentParser(description='Convert 34x34 PNG icon asset to C code')
parser.add_argument('-f', '--file', required=True, help='Input PNG file')
parser.add_argument('-n', '--name', required=True, help='Application name')
parser.add_argument('-o', '--output', required=True, help='Output C source file')

args = parser.parse_args()

if not os.path.isfile(args.file):
    sys.stderr.write(f"Input file not found: {args.file}\n")
    sys.exit(1)

icon_bytes = bytearray()

with Image.open(args.file) as img:
    width, height = img.size

    if (width, height) != (34, 34):
        sys.stderr.write(f"Input image must be 34x34 pixels, got {width}x{height}\n")
        sys.exit(1)

    for y in range(34):
        for x in range(0, 34, 8):
            byte = 0
            for bit in range(8):
                if x + bit < 34:
                    pixel = img.getpixel((x + bit, y))
                    if sum(pixel[:3]) < 384:
                        byte |= (1 << bit)
            icon_bytes.append(byte)

with open(args.output, 'w', encoding='utf-8') as f:
    f.write('#include "mailstation.h"\n\n')
    f.write('const ms_app_name_t ms_app_name = { 1, ' + str(len(args.name)) + ', 6, "' + args.name + '" };\n\n')
    f.write('const ms_app_icon_t ms_app_icon = { 173, 8, 0, 0, 34, 34, { ')
    for i in range(0, len(icon_bytes)):
        f.write(f'0x{icon_bytes[i]:02X}, ') 
    f.write('} };\n\n')
