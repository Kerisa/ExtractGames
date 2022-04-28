# -*- coding: utf-8 -*-

import os
import sys
from PIL import Image

# 合并 pfs 解出的带有 ipt 数据的 cg

def parse_ipt_meta(path):
    count = 0
    with open(path, 'rb') as f:
        for line in f.readlines():
            if line.find(b'id=') == -1:
                continue
            count += 1
    return count

def main():
    if len(sys.argv) != 2:
        print('usage: ', sys.argv[0], " <ipt file>")
        return
    count = parse_ipt_meta(sys.argv[1])
    if count == 0:
        print('[-] not found sub part, exit')
        return 0
    without_ext = os.path.splitext(sys.argv[1])[0]
    img = Image.new('RGBA', (0, 0))
    for i in range(count):
        tmp = img
        tmp2 = Image.open(without_ext + '_{:0>2d}'.format(i+1) + '.png')
        img = Image.new('RGBA', (tmp.size[0] + tmp2.size[0], tmp2.size[1]))
        img.paste(tmp, (0, 0), tmp)
        img.paste(tmp2, (tmp.size[0], 0), tmp2)
    img.save(without_ext + '_merge.png')

main()