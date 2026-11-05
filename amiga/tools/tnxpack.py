#!/usr/bin/env python3
"""tnxpack.py OUT.tnx FILE..: packs Tennix's data into tennix.tnx for the
Amiga build, as upstream's tnxar -cf does, in the layout the m68k build
reads.

Why not upstream's tnxar: the archive is the in-memory structs written
raw, and m68k-amigaos GCC aligns uint32_t on 2 bytes, x86 on 4. A
TennixArchiveItem is 100 bytes on a PC (2 padding bytes after the 86-byte
name) and 96 on the Amiga (name, offset at 86, length at 90, key at 94,
one padding byte). src/archive.h checks that size on the Amiga. Offsets
and lengths are big-endian (network order) on every machine.

PNGs without an alpha channel (colour types 0, 2, 3, any bit depth) are
stored as RGBA (colour type 6), same pixels: OpenGPU's SDL2_image.module
0.6 damages the host emulator's heap on some non-RGBA PNGs (rows not a
multiple of 4 bytes; see Team 2's PORT_BRIEF). The game converts every
picture to RGBA8888 after loading anyway.
The AmiCradle Team. GPL 2 or later, like Tennix."""
import os
import struct
import sys
import zlib

HEADER = b"thpinfo.com/2009/tennix/afmt "  # 29 chars + NUL = 30
MAXNAME = 86
ITEM_SIZE = 96  # m68k-amigaos layout
SIG = b"\x89PNG\r\n\x1a\n"


def chunks(data):
    i = 8
    while i < len(data):
        n, = struct.unpack(">I", data[i:i + 4])
        yield data[i + 4:i + 8], data[i + 8:i + 8 + n]
        i += 12 + n


def paeth(a, b, c):
    p = a + b - c
    pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
    return a if pa <= pb and pa <= pc else b if pb <= pc else c


def unfilter(raw, stride, h, bpp):
    rows, prev, pos = [], bytearray(stride), 0
    for _ in range(h):
        f = raw[pos]
        line = bytearray(raw[pos + 1:pos + 1 + stride])
        pos += stride + 1
        for i in range(stride):
            a = line[i - bpp] if i >= bpp else 0
            c = prev[i - bpp] if i >= bpp else 0
            b = prev[i]
            line[i] = (line[i] + [0, a, b, (a + b) // 2, paeth(a, b, c)][f]) & 255
        rows.append(line)
        prev = line
    return rows


def samples(line, w, depth, n):
    """the row's w*n samples, as 8-bit values (palette indices kept)"""
    if depth == 8:
        return list(line[:w * n])
    if depth == 16:
        return [line[i] for i in range(0, w * n * 2, 2)]
    per = 8 // depth
    mask = (1 << depth) - 1
    out = []
    for i in range(w * n):
        byte = line[i // per]
        out.append((byte >> (8 - depth * (i % per + 1))) & mask)
    return out


def to_rgba(data):
    cl = list(chunks(data))
    w, h, depth, ctype, comp, filt, interlace = struct.unpack(">IIBBBBB", cl[0][1])
    if ctype in (4, 6) or interlace:
        return None
    n = {0: 1, 2: 3, 3: 1}[ctype]
    stride = (w * n * depth + 7) // 8
    bpp = max(1, n * depth // 8)
    rows = unfilter(zlib.decompress(b"".join(d for t, d in cl if t == b"IDAT")), stride, h, bpp)
    plte = b"".join(d for t, d in cl if t == b"PLTE")
    trns = b"".join(d for t, d in cl if t == b"tRNS")
    scale = 255 // ((1 << depth) - 1) if depth < 8 else 1
    out = bytearray()
    for line in rows:
        s = samples(line, w, depth, n)
        out.append(0)  # filter: none
        for x in range(w):
            if ctype == 3:
                i = s[x]
                r, g, b = plte[i * 3:i * 3 + 3]
                a = trns[i] if i < len(trns) else 255
            elif ctype == 0:
                v = s[x] * scale
                r = g = b = v
                a = 0 if len(trns) == 2 and s[x] == (struct.unpack(">H", trns)[0] >> (16 - depth if depth == 16 else 0)) else 255
            else:
                r, g, b = s[x * 3:x * 3 + 3]
                a = 255
                if len(trns) == 6:
                    key = struct.unpack(">HHH", trns)
                    if depth == 16:
                        key = tuple(k >> 8 for k in key)
                    if (r, g, b) == key:
                        a = 0
            out += bytes((r, g, b, a))
    def chunk(t, d):
        return struct.pack(">I", len(d)) + t + d + struct.pack(">I", zlib.crc32(t + d) & 0xffffffff)
    png = bytearray(SIG)
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 6, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(bytes(out), 9))
    png += chunk(b"IEND", b"")
    return bytes(png)


def main():
    out, files = sys.argv[1], sorted(sys.argv[2:])
    blobs, names = [], []
    for path in files:
        data = open(path, "rb").read()
        name = os.path.basename(path)
        if data[:8] == SIG:
            rgba = to_rgba(data)
            if rgba is not None:
                print("tnxpack: %s stored as RGBA" % name)
                data = rgba
        assert len(name) < MAXNAME, name
        names.append(name.encode())
        blobs.append(data)
    count = len(files)
    assert count <= 255
    hkey = (0xaa + 0x77 * count * 3) % 0xff
    head = HEADER + b"\0" + struct.pack(">BBBB", 1, 0, hkey & 0xff, count)
    assert len(head) == 34
    offset = len(head) + count * ITEM_SIZE
    items, body = bytearray(), bytearray()
    for i, (name, data) in enumerate(zip(names, blobs)):
        key = (0xaa ^ ((i << 2) % 0x100)) & 0xff
        item = name.ljust(MAXNAME, b"\0") + struct.pack(">IIB", offset, len(data), key) + b"\0"
        assert len(item) == ITEM_SIZE
        items += bytes(c ^ hkey for c in item)
        body += bytes(c ^ key for c in data)
        offset += len(data)
    with open(out, "wb") as f:
        f.write(head + items + body)
    print("tnxpack: %s, %d files, %d bytes" % (out, count, offset))


if __name__ == "__main__":
    main()
