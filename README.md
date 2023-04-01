# DAT2 - Fallout DAT file packer/unpacker

A command-line tool for packing/unpacking Fallout 1/2 DAT files.

Original author: **Anchorite (TeamX)**

## Usage

    dat2 <command> [options] [-t dir] [-d dir] DAT_file [list | @response_file]

### Commands
    a: Add files to DAT file. Create new if DAT file does not exist
    x: Extract files from DAT file
    d: Delete files from DAT file (only info about files)
    l: List files in DAT file
    k: Shrink DAT file

### Options
    -s: create Fallout 1 DAT file
    -r: recurse into subdirectories
    -0..9: Compression method
           (Fallout1: 0 - store, other numbers - compress (default)
           (Fallout2: 0 - store, 1 - best speed, 9 - best compression (default)
    -p: extract without paths
    -d: extract files into specified directory
    -t: add files to specified directory of DAT file
    --: end of options
