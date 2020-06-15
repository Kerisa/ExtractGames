# -*- coding: utf-8 -*-

import os
import sys
from PIL import Image, ImageFile, BmpImagePlugin

_i16, _i32 = BmpImagePlugin.i16, BmpImagePlugin.i32

class BmpAlphaImageFile(ImageFile.ImageFile):
    format = "BMP+Alpha"
    format_description = "BMP with full alpha channel"

    def _open(self):
        s = self.fp.read(14)
        if s[:2] != b'BM':
            raise SyntaxError("Not a BMP file")
        offset = _i32(s[10:])

        self._read_bitmap(offset)

    def _read_bitmap(self, offset):

        s = self.fp.read(4)
        s += ImageFile._safe_read(self.fp, _i32(s) - 4)

        if len(s) not in (40, 108, 124):
            # Only accept BMP v3, v4, and v5.
            raise IOError("Unsupported BMP header type (%d)" % len(s))

        bpp = _i16(s[14:])
        if bpp != 32:
            # Only accept BMP with alpha.
            raise IOError("Unsupported BMP pixel depth (%d)" % bpp)

        compression = _i32(s[16:])
        if compression == 3:
            # BI_BITFIELDS compression
            mask = (_i32(self.fp.read(4)), _i32(self.fp.read(4)),
                    _i32(self.fp.read(4)), _i32(self.fp.read(4)))
            # XXX Handle mask.
        elif compression != 0:
            # Only accept uncompressed BMP.
            raise IOError("Unsupported BMP compression (%d)" % compression)

        self.mode, rawmode = 'RGBA', 'BGRA'

        self._size = (_i32(s[4:]), _i32(s[8:]))
        direction = -1
        if s[11] == '\xff':
            # upside-down storage
            self._size = self.size[0], 2**32 - self.size[1]
            direction = 0

        self.info["compression"] = compression

        # data descriptor
        self.tile = [("raw", (0, 0) + self.size, offset,
            (rawmode, 0, direction))]

