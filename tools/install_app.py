#!/usr/bin/python

import argparse
import sys
import os

parser = argparse.ArgumentParser(description='Install application to Mailstation dataflash file')
parser.add_argument('-f', '--file', required=True, help='Application file to install')
parser.add_argument('-d', '--dataflash', help='Location of the dataflash.bin file, default is current directory')
parser.add_argument('-n', '--index', type=int, choices=range(5), help='App index (0-4), default is 0')

args = parser.parse_args()

# Load the app file
try:
    with open(args.file, 'rb') as f:
        app_data = f.read()
    
    if len(app_data) > 16384:
        print(f"Error: Application file '{args.file}' is too large ({len(app_data)} bytes). Maximum size is 65536 bytes")
        sys.exit(1)
        
except FileNotFoundError:
    print(f"Error: Application file '{args.file}' not found")
    sys.exit(1)

# Load the dataflash file
dataflash_file = args.dataflash if args.dataflash else 'dataflash.bin'

try:
    with open(dataflash_file, 'rb') as f:
        dataflash_data = bytearray(f.read())
except FileNotFoundError:
    dataflash_data = bytearray(524288)  # Create a new empty dataflash if not found

# Get the index of the app to install, default 0
app_index = args.index if args.index is not None else 0

# Get the number of apps currently installed
num_apps = dataflash_data[0x20000]
print(f"Number of apps currently installed: {num_apps}")

if num_apps < app_index:
    print(f"Error: App index {app_index} is out of range (0-{num_apps})")
    sys.exit(1)

# Calculate the start address in dataflash for this app
dataflash_offset = 0x4000 * app_index

# Write the app data to dataflash
dataflash_data[dataflash_offset:dataflash_offset + len(app_data)] = app_data

# Update the number of apps if we're installing beyond current count
if app_index >= num_apps:
    dataflash_data[0x20000] = app_index + 1
    appinfo_index = 0x20004 + 6 * app_index
    dataflash_data[appinfo_index] = 0x00
    dataflash_data[appinfo_index + 1] = app_index
    dataflash_data[appinfo_index + 2] = 0x00
    dataflash_data[appinfo_index + 3] = 0x00
    dataflash_data[appinfo_index + 4] = 0x00
    dataflash_data[appinfo_index + 5] = app_index + 0x18

# Write the modified dataflash back to file
with open(dataflash_file, 'wb') as f:
    f.write(dataflash_data)

print(f"Successfully installed '{args.file}' to app index {app_index}")
