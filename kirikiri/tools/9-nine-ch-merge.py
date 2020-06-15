# -*- coding: utf-8 -*-

import alpha_bmp
import csv
import os
import pprint
import subprocess
import sys

from PIL import Image

CURDIR = ""
DATADIR = ""
DATAFILE = ""

# layer_type == 2 的项目中可作为差分的项，目前有以下两个（来自 9-nine），假定一个 txt 中只有一个项是差分
DIFF_NAME_LIST = ['表情', '目']

def make_fg(layer2_item, base_item, face_diff):
    base_file = DATADIR + '_' + base_item['layer_id'] + '.bmp'
    if os.path.isfile(base_file):
        print('[+] process base: ', base_file)
    else:
        print('[-] ', base_file , ' not exist')
        return
    
    if len(face_diff) == 0:
        base = alpha_bmp.BmpAlphaImageFile(base_file)
        save_path = DATADIR + '_' + layer2_item['name'] + '+' + base_item['name'] + '+' + base_item['layer_id'] + '.png'
        base.save(save_path, quality = 100)
        print('[+] save directly: ', save_path)
        return

    exe_handler = os.path.join(CURDIR, 'alpha-blend.exe')

    for face_item in face_diff:
        face_file = DATADIR + '_' + face_item['layer_id'] + '.bmp'
        if os.path.isfile(face_file):
            print('[+] process diff: ', face_file)
        else:
            print('[-] ', face_file , ' not exist')
            continue

        save_path = DATADIR + '_' + layer2_item['name'] + '+' + base_item['name'] + '+' + face_item['name'] + '.png'
        base = alpha_bmp.BmpAlphaImageFile(base_file)
        diff = alpha_bmp.BmpAlphaImageFile(face_file)
        baseX = int(base_item['left'])
        baseY = int(base_item['top'])

        pos_l = int(face_item['left']) - baseX
        pos_t = int(face_item['top']) - baseY
        assert pos_l + diff.size[0] < int(base_item['width'])
        assert pos_t + diff.size[1] < int(base_item['height'])

        diff2 = Image.new('RGBA', (base.size[0], base.size[1]))
        diff2.paste(diff, (pos_l, pos_t, pos_l + diff.size[0], pos_t + diff.size[1]), diff)

#        merge = Image.alpha_composite(base, diff2)
#        merge = Image.composite(diff2, base, diff2)
#        merge = Image.new('RGBA', (base.size[0], base.size[1]))
#        merge.paste(base, (0, 0))
#        merge.paste(diff, (pos_l, pos_t), diff)
#        merge.save(save_path, quality = 100)
        cmd = '"' + exe_handler + '" "' + base_file + '" "' + face_file + '" "' + save_path + '" ' + str(pos_l) + ' ' + str(pos_t)
        subprocess.Popen(cmd)
        print('[+] save to: ', save_path)

def parse_csv(datafile):
    data = []
    n = 0
    with open(datafile, "r", encoding='utf-16-le') as sd2:
        r = csv.DictReader(sd2)
        for line in r:
            kv = {}
            for k,v in line.items():
                k1 = k.split('\t')
                v1 = v.split('\t')
                assert len(k1) == len(v1)
                for i in range(len(k1)):
                    if len(k1[i]) == 0:
                        continue
                    if k1[i] == "#layer_type" and len(v1[i]) == 0:
                        continue
                    kv[k1[i]] = v1[i]
            if '#layer_type' in kv:
                data.append(kv)

    return data

def find_item(group, key, val):
    gg = {}
    for k,v in group.items():
        if v[key] == val:
            gg[k] = v

    return gg

def process(layer_2, layer_0):
#    pprint.pprint(layer_0)
#    pprint.pprint(layer_2)

    tmp = {}
    
    # 查找 layer_type 为 2 的项中可作为差分的项，假定只有一个
    for d in DIFF_NAME_LIST:
        tmp = find_item(layer_2, 'name', d)
        if len(tmp) > 0:
            break
    assert len(tmp) <= 1

    # 没有差分需要处理
    if len(tmp) == 1:
        face_gid = list(tmp.values())[0]['layer_id']
        print('face_gid:', face_gid)
    else:
        print('[+] diff image not found, skip')
        face_gid = 0

    for _,L2 in layer_2.items():
        if L2['layer_id'] == face_gid:
            continue

        # 找底图，一般就是唯一一个
        tmp = find_item(layer_0, 'group_layer_id', L2['layer_id'])
        base_item = list(tmp.values())

        tmp = find_item(layer_0, 'group_layer_id', face_gid)
        face_diff = list(tmp.values())

        for b in base_item:
            make_fg(L2, b, face_diff)

def main():
    if len(sys.argv) < 2:
        print('usage: ' + sys.argv[0] + ' <intput_txt>')
        return

    global CURDIR
    CURDIR = os.path.dirname(sys.argv[0])

    global DATAFILE
    global DATADIR
    DATADIR, DATAFILE = os.path.splitext(sys.argv[1])

    groups = parse_csv(sys.argv[1])
    layer_0 = {}
    layer_2 = {}
    for g in groups:
        if g['#layer_type'] == '0':
            layer_0[g['layer_id']] = g
        elif g['#layer_type'] == '2':
            layer_2[g['layer_id']] = g
        else:
            raise 'unknown layer_type'

    process(layer_2, layer_0)

if __name__ == '__main__':
    main()
