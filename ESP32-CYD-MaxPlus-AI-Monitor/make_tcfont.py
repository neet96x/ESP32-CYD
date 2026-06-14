"""
สร้าง tcUnicode font header (.h) สำหรับ tcUnicodeHelper + TFT_eSPI
UnicodeFontGlyph: {relativeChar, relativeBmpOffset, width, height, xAdvance, xOffset(int8), yOffset(int8)}
"""
from PIL import Image, ImageDraw, ImageFont
import os

FONT_FILE = "NotoSansThai.ttf"
FONT_SIZE = 24
OUT_H     = "src/NotoSansThai24_tc.h"
FONT_NAME = "NotoSansThai24"

BLOCKS = [
    (0x0020, list(range(0x0020, 0x007F))),  # ASCII
    (0x0E01, list(range(0x0E01, 0x0E5C))),  # Thai
]

def make_h(font_path, size, blocks_def, out_path, name):
    pil = ImageFont.truetype(font_path, size)
    ascent, descent = pil.getmetrics()
    yAdvance = ascent + descent

    all_blocks = []
    for base_cp, codepoints in blocks_def:
        gdata, bmp_bytes, bmp_offsets = [], [], []
        offset = 0
        for cp in codepoints:
            ch   = chr(cp)
            bbox = pil.getbbox(ch) or (0, 0, 1, 1)
            l, t, r, b = bbox
            w, h = max(r-l, 1), max(b-t, 1)
            img  = Image.new("1", (w, h), 0)
            ImageDraw.Draw(img).text((-l, -t), ch, font=pil, fill=1)
            # tcUnicode ต้องการ bits ต่อกันแบบไม่ pad per row
            pixels = list(img.getdata())
            packed = []
            for i in range(0, len(pixels), 8):
                byte = 0
                for j in range(8):
                    if i+j < len(pixels) and pixels[i+j]:
                        byte |= (1 << (7-j))
                packed.append(byte)
            raw  = packed
            gdata.append(dict(cp=cp, w=w, h=h,
                              adv=round(pil.getlength(ch)),
                              xOff=l, yOff=t - ascent,
                              raw=raw))
            bmp_offsets.append(offset)
            bmp_bytes.extend(raw)
            offset += len(raw)
        all_blocks.append((base_cp, gdata, bmp_bytes, bmp_offsets))

    os.makedirs(os.path.dirname(out_path) or ".", exist_ok=True)
    lines = [
        f"// tcUnicode font: {name}, {size}px  (ASCII + Thai)",
        "#pragma once",
        "#include <UnicodeFontDefs.h>",
        "",
    ]

    for idx, (base_cp, gdata, bmp, offsets) in enumerate(all_blocks):
        bname = f"{name}_blk{idx}"

        # bitmap
        lines.append(f"static const uint8_t {bname}_bmp[] PROGMEM = {{")
        row = []
        for byte in bmp:
            row.append(f"0x{byte:02X}")
            if len(row) == 16:
                lines.append("  " + ",".join(row) + ",")
                row = []
        if row:
            lines.append("  " + ",".join(row))
        lines.append("};")
        lines.append("")

        # glyphs — field order: relativeChar, relativeBmpOffset, width, height, xAdvance, xOffset(int8), yOffset(int8)
        lines.append(f"static const UnicodeFontGlyph {bname}_glyphs[] PROGMEM = {{")
        for i, g in enumerate(gdata):
            rel  = g['cp'] - base_cp
            xOff = g['xOff']
            yOff = g['yOff']  # negative = above baseline
            lines.append(f"  {{{rel},{offsets[i]},{g['w']},{g['h']},{g['adv']},"
                         f"(int8_t){xOff},(int8_t){yOff}}},  // U+{g['cp']:04X}")
        lines.append("};")
        lines.append("")

        lines.append(f"static const UnicodeFontBlock {bname} PROGMEM = {{")
        lines.append(f"  {base_cp}, {bname}_bmp, {bname}_glyphs, {len(gdata)}")
        lines.append("};")
        lines.append("")

    # blocks array — descending startingNum
    lines.append(f"static const UnicodeFontBlock {name}_blocks[] PROGMEM = {{")
    for idx in reversed(range(len(all_blocks))):
        lines.append(f"  {name}_blk{idx},")
    lines.append("};")
    lines.append("")

    lines.append(f"static const UnicodeFont {name} PROGMEM = {{")
    lines.append(f"  {name}_blocks, {len(all_blocks)}, {yAdvance}, TCFONT_ONE_BIT_PER_PIXEL")
    lines.append("};")

    with open(out_path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines))

    total_bmp   = sum(len(b[2]) for b in all_blocks)
    total_glyph = sum(len(b[1]) for b in all_blocks)
    print(f"Done: {out_path}  ({total_glyph} glyphs, {total_bmp} bytes bitmap)")

make_h(FONT_FILE, FONT_SIZE, BLOCKS, OUT_H, FONT_NAME)
