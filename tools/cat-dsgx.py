#!/usr/bin/env python
from __future__ import print_function
import hashlib, struct, sys
from collections import namedtuple

Chunk = namedtuple('Chunk', 'kind size name payload')
DsgxPayload = namedtuple('DsgxPayload', 'word_count commands')

def main(filenames):
    for filename in filenames:
        with open(filename) as dsgx_file:
            cat_dsgx(dsgx_file.read())

def cat_dsgx(contents):
    for chunk in chunks(contents):
        print(chunk.kind, chunk.size, chunk.name, chunk.payload)

def chunks(contents):
    header_size = 8
    word_size = 4
    offset = 0
    while True:
        kind, size, name = struct.unpack("<4sI32s", contents[offset:offset + header_size + 32])
        name = name[:name.find('\x00')]
        yield Chunk(kind, size, name, hashlib.md5(contents[offset + header_size:offset + header_size + size * 4]).hexdigest())
        offset += header_size + size * word_size
        if len(contents) <= offset:
            break

def extract_dsgx_payload(contents):
    pass

payload_extractors = {
    'DSGX': extract_dsgx_payload
}

if __name__ == '__main__':
    main([sys.argv[1]])
