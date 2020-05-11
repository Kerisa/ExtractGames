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


def SpiltGroup(file_list):
    groups = {}
    for f in file_list:
        idx = f.find('base.')
        if idx != -1:
            groups[f] = []
            for ff in file_list:
                if ff[:idx] == f[:idx] and ff != f:
                    groups[f].append(ff)
    return groups

def ProcessGroup(output_dir, dir, base_file, diff_file_list):
    print('[+] group: ', base_file, diff_file_list)
    count = 0
    for diff_file in diff_file_list:
        print('[+] process: ', diff_file)
        save_path = os.path.join(output_dir, base_file[:base_file.rfind('.')] + '_' + str(count) + '.png')
        base = BmpAlphaImageFile(os.path.join(dir, base_file))
        diff = BmpAlphaImageFile(os.path.join(dir, diff_file))
        merge = Image.new('RGBA', (max(base.size[0], diff.size[0]), max(base.size[1], diff.size[1])))
        merge.paste(base, (0, 0, base.size[0], base.size[1]))
        merge.paste(diff, (0, 0, diff.size[0], diff.size[1]), diff)
        merge.save(save_path, quality = 100)
        print('[+] save to: ', save_path)
        count += 1

def main():
    if len(sys.argv) <= 1:
        print("Usage: sekaichu-ch-merge.py <input-dir>")
        return 1

    input_folder = sys.argv[1]
    all_files = os.walk(input_folder)
    for path, dir_list, file_list in all_files:
        if len(file_list) == 0:
            continue
        print(path)
        file_groups = SpiltGroup(file_list)
        for k, v in file_groups.items():
            output_dir = os.path.join(path, 'output_0')
            if not os.path.exists(output_dir):
                os.mkdir(output_dir)
            ProcessGroup(output_dir, path, k, v)


if __name__ == "__main__":
    Image.register_open(BmpAlphaImageFile.format, BmpAlphaImageFile)
    Image.register_extension(BmpAlphaImageFile.format, ".bmp")
    main()

