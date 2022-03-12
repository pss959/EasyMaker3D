#!/usr/bin/env python3

from sys import argv

from BuildMenuIcons.IconBuilder import IconBuilder

# -----------------------------------------------------------------------------
# Mainline.
# -----------------------------------------------------------------------------

usage = f'*** Usage: {argv[0]} list|build'

def main():
    if len(argv) != 2:
        print(usage)
        return 1

    builder = IconBuilder()
    cmd = argv[1]
    if cmd == 'list':
        builder.List()
    elif cmd == 'build':
        builder.BuildAll()
    else:
        print(usage)
        return 1

if __name__ == '__main__':
    main()
