#!/usr/bin/env python3
# save as pad_files.py - Works on Windows, Linux, macOS

import sys
import os
import argparse

def hex_to_int(value):
    """Convert hex string to integer."""
    if isinstance(value, str):
        if value.lower().startswith('0x'):
            return int(value, 16)
    return int(value)

def pad_and_concatenate(output_file, target_address, input_file1, input_file2, max_size=0):
    """Concatenate files and pad to target address."""

    pad_byte=0x00

    # Open input files
    file1_bytes = bytearray()
    if not os.path.exists(input_file1):
        print(f"Error: File not found: {input_file1}")
        return False

    with open(input_file1, 'rb') as f:
        file1_bytes.extend(f.read())

    file2_bytes = bytearray()
    if not os.path.exists(input_file2):
        print(f"Error: File not found: {input_file2}")
        return False

    with open(input_file2, 'rb') as f:
        file2_bytes.extend(f.read())

    # Check if files exceed final desired size
    file1_size = len(file1_bytes)
    file2_size = len(file2_bytes)
    target_addr = hex_to_int(target_address)
    max_size = hex_to_int(max_size)

    print(f'File1 is {file1_size} (0x{file1_size:X}) bytes long, File2 is {file2_size} (0x{file2_size:X}) bytes long')

    if file1_size > target_addr:
        print(f"Error: File1 exceed target address ({file1_size} > {target_addr})")
        return False
    else:
        padding_size = target_addr - file1_size

    file_size = file1_size + padding_size + file2_size

    if max_size:
        if file_size > max_size:
            print(f'Error: Files + padding size ({padding_size} bytes) exceeds maximum size, aborting')
            return False

    # Add padding
    if padding_size > 0:
        padding = bytes([pad_byte] * padding_size)
        file1_bytes.extend(padding)
        print(f"Added {padding_size} bytes of padding")

    file1_bytes.extend(file2_bytes)

    # Write output
    with open(output_file, 'wb') as f:
        f.write(file1_bytes)

    # Verify
    print(f"Created: {output_file}")
    print(f"Target address: 0x{target_addr:X} ({target_addr} bytes)")
    print(f"Final size: 0x{len(file1_bytes):X} ({len(file1_bytes)} bytes)")

    return True

def main():
    parser = argparse.ArgumentParser(description='Concatenate and pad binary files')
    parser.add_argument('-o', '--output', required=True, help='Output file name')
    parser.add_argument('-a', '--address', required=True, help='Target address (decimal or 0x hex)')
    parser.add_argument('-s', '--maxsize', help='Desired maximum size for result binary')
    parser.add_argument('--file1', required=True, help='First input file to concatenate')
    parser.add_argument('--file2', required=True, help='Second input file to concatenate')

    args = parser.parse_args()

    # Sanity check final addr vs target sewing address
    if args.maxsize:
        if args.address > args.maxsize:
            print("Error: concatenation address exceeds desired final address")
            return 1

    # Process files
    success = pad_and_concatenate(
        args.output,
        args.address,
        args.file1,
        args.file2,
        args.maxsize
    )

    return 0 if success else 1

if __name__ == '__main__':
    sys.exit(main())

# Usage:
# python3 filesewing.py -o your-output.bin -a 0x2e000 -s 0x40000 --file1 your-file-1.bin --file2 your-file-2.bin
