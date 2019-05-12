# -*- coding: utf-8 -*-

import os
import sys
from PIL import Image

# 使用 pip install pillow 安装 pillow

# 合并 qlie 解包产生的 png 文件
# merge.py <目录名>

filePath = sys.argv[1]
group = {}

def init_group():
    count = 0
    for f in os.listdir(filePath):
        if os.path.splitext(f)[-1][1:] != 'png':
            continue

        if (len(f.split('+')) != 3):
            continue

        count += 1
        finalName = f[:f.find('+')]
        if group.get(finalName) == None:
            group[finalName] = []
        group[finalName].append(f)

    print('file:', count, 'group:', len(group))

def main():
    if len(sys.argv) != 2:
        print("usage: merge.py <directory>")
        return

    init_group()
    if len(group) == 0:
        print('empty matched file')
        return

    count = 1
    for k in group:
        print('process ' + str(count) + ': ' + k)
        lst = group[k]
        sizeX = 0
        sizeY = 0
        for i in lst:            
            img = Image.open(os.path.join(filePath, i))
            w, h = img.size
            sizeX = max(sizeX, w + int(i[i.rfind('x')+1:i.rfind('y')]))
            sizeY = max(sizeY, h + int(i[i.rfind('y')+1:i.rfind('.')]))
            
        print('size:', sizeX, sizeY)
        merge = Image.new('RGBA', (sizeX, sizeY))
        for i in lst:
            img = Image.open(os.path.join(filePath, i))
            x = int(i[i.rfind('x')+1:i.rfind('y')])
            y = int(i[i.rfind('y')+1:i.rfind('.')])
            w, h = img.size
            merge.paste(img, (x, y, x+w, y+h))
        merge.save(os.path.join(filePath, k + '.png'), quality = 100)
        count += 1


if __name__ == "__main__":
    main()