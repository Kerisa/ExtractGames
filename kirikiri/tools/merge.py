# -*- coding: utf-8 -*-
import json
import os
import sys
from PIL import Image


# 魔女的夜宴 cg 合并
# 1、使用 EMoteDumperXmoe.exe 解出 pimg 的 psb 文件后得到 XXX.psb.tjs 和 XXX 文件夹
# 2、将 tlg 文件转成 png 图像保存在原位置
# 3、执行 "Merge.py XXX.psb.tjs" 进行合并

tjs_name = sys.argv[1]
folder = tjs_name[:tjs_name.find('.')]
json_name = folder + '.json'

f = open(tjs_name, 'r')
f.seek(0, 2)
file_len = f.tell()
f.seek(0, 0)
data = f.read(file_len)
data = data.replace('=>', ':').replace('(const)', '')
content = list(data)
data_len = len(data)

idx_l = 0
idx_r = data_len
while True:
    res_l = data.find('[', idx_l)
    res_r = data.rfind(']', 0, idx_r)
    if res_l == -1 or res_r == -1:
        break

    if data[res_l-1] == '%':
        content[res_l-1] = ' '
        content[res_l] = '{'
        content[res_r] = '}'
    idx_l = res_l + 1
    idx_r = res_r - 1

    if idx_l >= data_len or idx_r >= data_len:
        break


f2 = open(json_name, 'w')
f2.write(''.join(content))
f2.close()
j = json.loads(''.join(content))

# find base
baseImg = {}
for item in j['layers']:
    if item['height'] == j['height'] and item['width'] == j['width']:
        baseImg = item

for item in j['layers']:
    diff = Image.open(folder + '\\' + str(item['layer_id']) + '.png')
    base = Image.open(folder + '\\' + str(baseImg['layer_id']) + '.png')
    merge = Image.new('RGBA', (baseImg['width'], baseImg['height']))
    merge.paste(base, (0, 0, int(baseImg['width']), int(baseImg['height'])))
    left = item['left']
    top = item['top']
    right = left + item['width']
    bottom = top + item['height']
    merge.paste(diff, (left, top, right, bottom), diff)
    merge.save(folder + '\\merge_' + str(item['layer_id']) + '.png', quality = 100)






