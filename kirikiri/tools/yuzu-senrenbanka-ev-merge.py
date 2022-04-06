# -*- coding: utf-8 -*-
import json
import os
import sys
from PIL import Image
import alpha_bmp

TJSDIR = ""
BMPDIR = ""
TJSBASE = ""

# 千恋*万花 cg 合并
# 1、使用 EMoteDumperXmoe.exe 解出 pimg 的 psb 文件后得到 XXX.psb.tjs 和 XXX 文件夹
# 2、执行 "yuzu-senrenbanka-ev-merge.py XXX.psb.tjs" 进行合并

def getCommonPrefix(s1, s2):
    length = 1
    while length < len(s1) and s1[:length] == s2[:length]:
        length = length + 1
    return length - 1

def findBaseOfDiff(baseGroup, diff):
    for b in baseGroup:
        if getCommonPrefix(b['name'], diff['name']) > 0:
            return b
    return None

def ConvertToJson(tjs_data):
    tjs_data = tjs_data.replace('=>', ':').replace('(const)', '')
    content = list(tjs_data)
    data_len = len(tjs_data)

    idx_l = 0
    idx_r = data_len
    while True:
        res_l = tjs_data.find('[', idx_l)
        res_r = tjs_data.rfind(']', 0, idx_r)
        if res_l == -1 or res_r == -1:
            break

        if tjs_data[res_l-1] == '%':
            content[res_l-1] = ' '
            content[res_l] = '{'
            content[res_r] = '}'
        idx_l = res_l + 1
        idx_r = res_r - 1

        if idx_l >= data_len or idx_r >= data_len:
            break
    return content

def FindXa(j, X):
    for item in j['layers']:
        n = item['name']
        if len(n) == 2 and n[0] == X and n[1] == 'a':
            return item
    return None

def main():
    tjs_name = sys.argv[1]

    global TJSDIR
    global BMPDIR
    global TJSBASE
    (TJSDIR,tempfilename) = os.path.split(tjs_name)
    TJSBASE = tempfilename[:tempfilename.find('.')]
    BMPDIR = os.path.join(TJSDIR, TJSBASE)

    j = {}
    with open(tjs_name, 'r') as f:
        data = f.read()
        content = ConvertToJson(data)
        with open(BMPDIR + '.json', 'w') as f2:
            f2.write(''.join(content))
            j = json.loads(''.join(content))

    for item in j['layers']:
        baseItem = FindXa(j, item['name'][0])
        diff = alpha_bmp.BmpAlphaImageFile(os.path.join(BMPDIR, str(item['layer_id']) + '.bmp'))
        base = alpha_bmp.BmpAlphaImageFile(os.path.join(BMPDIR, str(baseItem['layer_id']) + '.bmp'))
        merge = Image.new('RGBA', (baseItem['width'], baseItem['height']))
        merge.paste(base, (0, 0, int(baseItem['width']), int(baseItem['height'])))
        left = item['left']
        top = item['top']
        right = left + diff.size[0]
        bottom = top + diff.size[1]
        merge.paste(diff, (left, top, right, bottom), diff)
        merge.save(os.path.join(BMPDIR, TJSBASE + '_merge_' + '{0:05}'.format(item['layer_id']) + '.png'), quality = 100)

if __name__ == '__main__':
    main()





