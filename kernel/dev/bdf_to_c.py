
### ORIGINAL 'conv_bdf2c' which converts bdf2c output to a more minimal format.
# import re
# s = ""
# with open('font.c', 'r') as f: s = f.read()

# W, H = 16, 16
# Wm = re.search(r'\.Width = ([0-9]+)', s)
# Hm = re.search(r'\.Height = ([0-9]+)', s)
# if Wm: W = int(Wm[1])
# if Hm: H = int(Hm[1])
# Wn = ((W + 7) & -8) // 8

# for i in range(0x100):
#     v = bin(i)[2:].replace('1', 'X').replace('0', '_')
#     v = (8 - len(v)) * '_' + v
#     x = hex(i).upper()[2:]
#     x = '0x' + (2 - len(x)) * '0' + x
#     print(v, x)
#     s = s.replace(v, x)

# s = re.sub(r'^[^\n]*_widths__\[\] = \{[^}]*\};', '', s, 1, re.DOTALL | re.MULTILINE)
# s = re.sub(r'^[^\n]*_index__\[\] = \{[^}]*\};', '', s, 1, re.DOTALL | re.MULTILINE)
# s = re.sub(r'const struct bitmap_font [a-zA-Z_$0-9]+ = \{[^}]*\};', '', s, 1, re.DOTALL)
# s = re.sub(r'^\s*//.*$', '', s, flags=re.M)
# s = re.sub(r'^#include.*$', '', s, flags=re.M)
# s = re.sub(r'static const unsigned char __([a-zA-Z0-9_]+)_bitmap__\[\]',
#     lambda m: f'unsigned char {m[1]}[][{Wn * H}]', s)
# s = re.sub(r'^\n+', '', s)
# s = re.sub(r'\n+$', '', s)
# s = re.sub(r'{\n\n\n', '{\n{', s)
# s = re.sub(r',\n\n\n', '},{\n', s)
# s = re.sub(r',\n}', '}\n}', s)

# with open('font.out.c', 'w') as f: f.write(s)

from os import replace
from pickle import FALSE
from re import S
import sys
chars = {}
font = { "x": 0, "y": 0, "w": 16, "h": 16 }
with open(sys.argv[1]) as fin:
    char = 0
    bm = False
    bm_n = 0
    c = False
    for line in fin:
        match line.split():
            case ['FONTBOUNDINGBOX', W, H, X, Y]:
                font["x"] = int(X)
                font["y"] = int(Y)
            case ['ENCODING', U]:
                # if not U.startswith('U+'): c = False; continue
                c = True
                bm = False
                bm_n = 0
                char = int(U, base=10)
                if char > 127: break # no more chars
                chars[char] = ({})
            case ['BBX', W, H, X, Y] if c:
                chars[char]["W"] = int(W)
                chars[char]["H"] = int(H)
                chars[char]["X"] = int(X)
                chars[char]["Y"] = int(Y)
            case ['BITMAP'] if c:
                chars[char]["L"] = chars[char]["H"] * (((chars[char]["W"] + 7) & -8) // 8)
                chars[char]["B"] = chars[char]["L"] * [0]
                bm = True
                bm_n = 0
            case ['ENDCHAR']:
                bm = False
                bm_n = 0
            case [value] if bm and c:
                if bm_n >= chars[char]["L"]:
                    bm = False
                    bm_n = 0
                else:
                    chars[char]["B"][bm_n] = int(value, base=16)
                    bm_n += 1
            case _: continue

maxH = 0
for char in chars.values():
    mx = char["X"] + font["x"]
    my = char["Y"] + font["y"]
    for i in range(len(char["B"])):
        if mx > 0: char["B"][i] >>= +mx
        if mx < 0: char["B"][i] = min(255, char["B"][i] << -mx)
    if my > 0: char["B"] = [0] * my + char["B"][:-my]
    if my < 0: char["B"] = char["B"][-my:] + [0] *-my
    if len(char["B"]) > maxH: maxH = len(char["B"])

def conv(x: int, n: int = 2) -> str:
    s = hex(x).upper()[2:]
    return '0x' + '0' * (n - len(s)) + s

with open(sys.argv[2], 'w') as fout:
    fout.write('unsigned char ' + sys.argv[3] + f"[128][{maxH}] = {{\n")
    for idx, char in chars.items():
        fout.write('    { ')
        if len(char["B"]) != 0:
            fout.write(', '.join(map(conv, char["B"])))
            for i in range(maxH - len(char["B"])): fout.write(', 0x00')
        else:
            fout.write('0x00')
            for i in range(maxH - 1): fout.write(', 0x00')
        if chr(idx).isprintable(): t = f', \'{chr(idx)}\''
        else: t = ''
        fout.write(f' }}, // {idx}{t}\n')
    fout.write('};\n')
