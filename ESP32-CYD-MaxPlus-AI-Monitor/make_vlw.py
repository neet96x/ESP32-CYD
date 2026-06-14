"""
สร้าง .vlw font file สำหรับ TFT_eSPI Smooth Font
Header: 6×int32  (gCount, version=1, yAdvance, reserved, ascent, descent)
Glyph:  7×int32  (unicode, height, width, xAdvance, gdY, gdX, ignored)
gdY = distance from top of glyph bitmap to baseline (= ascent - bbox.top)
"""
import struct, os
from PIL import Image, ImageDraw, ImageFont

FONT_FILE = "NotoSansThai.ttf"

CODEPOINTS = (
    list(range(0x0020, 0x007F)) +
    list(range(0x0E01, 0x0E3B)) +
    list(range(0x0E40, 0x0E5C))
)

def make_vlw(font_path, size, codepoints, out_path):
    pil_font = ImageFont.truetype(font_path, size)
    ascent, descent = pil_font.getmetrics()   # PIL ascent = pixels above baseline

    glyphs = []
    for cp in codepoints:
        ch = chr(cp)
        bbox = pil_font.getbbox(ch)           # (left, top, right, bottom) relative to origin
        if bbox is None:
            continue
        l, t, r, b = bbox
        w, h = max(r - l, 1), max(b - t, 1)

        img = Image.new("L", (w, h), 0)
        ImageDraw.Draw(img).text((-l, -t), ch, font=pil_font, fill=255)

        gdY = ascent - t    # pixels from bitmap top edge down to baseline
        gdX = l             # pixels from cursor X to left edge of bitmap
        adv = round(pil_font.getlength(ch))
        glyphs.append((cp, img, adv, gdY, gdX))

    os.makedirs(os.path.dirname(out_path) or ".", exist_ok=True)
    with open(out_path, "wb") as f:
        # Header: 6 × int32 big-endian
        f.write(struct.pack(">iiiiii",
            len(glyphs), 1, size, 0, ascent, descent))

        # Per-glyph: 7 × int32 big-endian
        for cp, img, adv, gdY, gdX in glyphs:
            w, h = img.size
            f.write(struct.pack(">iiiiiii", cp, h, w, adv, gdY, gdX, 0))

        # Bitmap data
        for cp, img, adv, gdY, gdX in glyphs:
            f.write(img.tobytes())

    print(f"Done: {out_path}  ({len(glyphs)} glyphs, {os.path.getsize(out_path):,} bytes)")

make_vlw(FONT_FILE, 24, CODEPOINTS, "data/NotoSansThai24.vlw")
make_vlw(FONT_FILE, 18, CODEPOINTS, "data/NotoSansThai18.vlw")
