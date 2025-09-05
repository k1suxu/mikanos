#include "font.hpp"

// 縦16(=2^4) * 横8(=2^3)でフォントを作成(2の冪場が嬉しい)
const uint8_t kFontA[16] = {
  0b00000000, //
  0b00011000, //    **
  0b00011000, //    **
  0b00011000, //    **
  0b00011000, //    **
  0b00100100, //   *  *
  0b00100100, //   *  *
  0b00100100, //   *  *
  0b00100100, //   *  *
  0b01111110, //  ******
  0b01000010, //  *    *
  0b01000010, //  *    *
  0b01000010, //  *    *
  0b11100111, // ***  ***
  0b00000000, //
  0b00000000, //
};


void WriteAscii(PixelWriter& writer, int x, int y, char c, const PixelColor& color) {
  // 今は'A'のフォントしか用意してない
  if (c != 'A') {
    return;
  }
  for (int dy = 0; dy < 16; dy++) {
    for (int dx = 0; dx < 8; dx++) {
      if ((kFontA[dy] << dx) & 0x80u) {
        writer.Write(x + dx, y + dy, color);
      }
    }
  }
}