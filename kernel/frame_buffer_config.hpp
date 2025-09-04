#pragma once

#include <stdint.h>

enum PixelFormat {
  kPixelRGBResv8BitPerColor,
  kPixelBGRResv8BitPerColor,
};

struct FrameBufferConfig {
  uint8_t* frame_buffer;          // フレームバッファの先頭アドレス
  uint32_t pixels_per_scan_line;  // 余白を含む横幅全体のピクセル数
  uint32_t horizontal_resolution; // 水平方向の解像度
  uint32_t vertical_resolution;   // 垂直方向の解像度
  enum PixelFormat pixel_format;  // 色指定のフォーマット
};
