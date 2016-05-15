#!/usr/bin/env python

import collections, struct, sys

#Chunk = collections.namedtuple('Chunk', '

def main(filenames):
    for filename in filenames:
        with open(filename) as dsgx_file:
            cat_dsgx(dsgx_file.read())

def cat_dsgx(contents):
    for chunk in chunks(contents):
        print(chunk)

def chunks(contents):
    offset = 0
    while True:
        name, size = struct.unpack("<4sI", contents[offset:offset + 8])
        yield (name, size)
        offset += 8 + size * 4
        if len(contents) <= offset:
            break

if __name__ == '__main__':
    main([sys.argv[1]])
