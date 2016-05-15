#!/usr/bin/env python
from __future__ import print_function
import collections, struct, sys

Chunk = collections.namedtuple('Chunk', 'kind size name')

def main(filenames):
    for filename in filenames:
        with open(filename) as dsgx_file:
            cat_dsgx(dsgx_file.read())

def cat_dsgx(contents):
    for chunk in chunks(contents):
        print(chunk.kind, chunk.size, chunk.name)

def chunks(contents):
    offset = 0
    while True:
        kind, size, name = struct.unpack("<4sI32s", contents[offset:offset + 8 + 32])
        name = name[:name.find('\x00')]
        yield Chunk(kind, size, name)
        offset += 8 + size * 4
        if len(contents) <= offset:
            break

if __name__ == '__main__':
    main([sys.argv[1]])
