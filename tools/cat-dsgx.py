#!/usr/bin/env python
from __future__ import print_function
import hashlib, struct, sys
from collections import namedtuple

word_size = 4

Chunk = namedtuple('Chunk', 'kind size name checksum payload')
DsgxPayload = namedtuple('DsgxPayload', 'word_count commands')
BsphPayload = namedtuple('BsphPayload', 'x y z radius')
CostPayload = namedtuple('CostPayload', 'polygons gpu_cycles')

def main(filenames):
    for filename in filenames:
        with open(filename, 'rb') as dsgx_file:
            cat_dsgx(dsgx_file.read())

def cat_dsgx(contents):
    for chunk in chunks(contents):
        print(chunk.kind, chunk.size, chunk.name, chunk.payload)

def chunks(contents):
    header_size = 8
    name_size = 32

    offset = 0
    while True:
        kind, size, name = struct.unpack('<4sI32s', contents[offset:offset + header_size + 32])
        name = rstrip_nulls(name) #name[:name.find('\x00')]
        payload_hash = hashlib.md5(contents[offset + header_size:offset + header_size + size * word_size]).hexdigest()
        payload = payload_extractors.get(kind, lambda _: None)(contents[offset + header_size + name_size:offset + header_size + size * word_size])
        yield Chunk(kind, size, name, payload_hash, payload)

        offset += header_size + size * word_size
        if len(contents) <= offset:
            break

def rstrip_nulls(string):
    return string[:string.find(b'\x00')]

def extract_dsgx_payload(contents):
    display_list_word_count, = struct.unpack('<I', contents[:word_size])
    commands = contents[word_size:]
    return DsgxPayload(display_list_word_count, commands)

def extract_bsph_payload(contents):
    x, y, z, r = struct.unpack('<iiii', contents)
    x /= pow(2, 12)
    y /= pow(2, 12)
    z /= pow(2, 12)
    r /= pow(2, 12)
    return BsphPayload(x, y, z, r)

def extract_cost_payload(contents):
    polygon_cost, gpu_cycle_cost = struct.unpack('<II',  contents)
    return CostPayload(polygon_cost, gpu_cycle_cost)

def extract_txtr_payload(contents):
    texture_count = struct.unpack('<I', contents[:word_size])
    pass

payload_extractors = {
    'DSGX': extract_dsgx_payload,
    'BSPH': extract_bsph_payload,
    'COST': extract_cost_payload,
    'TXTR': extract_txtr_payload
}

if __name__ == '__main__':
    main([sys.argv[1]])
