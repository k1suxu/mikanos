#!/usr/bin/python3

import argparse
import collections
import functools
import re
import sys

# . or * or @ のみで構成される1文字以上の文字列
BITMAP_PATTERN = re.compile(r'([.*@]+)')


def compile(src: str) -> bytes:
    src = src.lstrip()
    result = []

    for line in src.splitlines():
        m = BITMAP_PATTERN.match(line)
        if not m:
            continue

        # . => 0, otherwise => 1として、これを整数に埋め込みlittle-endianでバイト列として.oファイルに書き込み
        bits = [(0 if x == '.' else 1) for x in m.group(1)]
        bits_int = functools.reduce(lambda a, b: 2*a + b, bits)
        result.append(bits_int.to_bytes(1, byteorder='little'))

    return b''.join(result)


def main():
    # 必須: font → 入力のフォントファイル（テキスト）
    # オプション: -o → 出力ファイル名（デフォルト font.out）
    parser = argparse.ArgumentParser()
    parser.add_argument('font', help='path to a font file')
    parser.add_argument('-o', help='path to an output file', default='font.out')
    ns = parser.parse_args()

    with open(ns.o, 'wb') as out, open(ns.font) as font:
        src = font.read()
        out.write(compile(src))


if __name__ == '__main__':
    main()
