/**
 * @file main.cpp
 * 
 * カーネル本体のプログラムを書いたファイル.
 */

#include <cstdint>
#include <cstddef>

#include "frame_buffer_config.hpp"

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

struct PixelColor {
  uint8_t r, g, b;
};

/**
 * @class PixelWriter : ピクセル描画用の抽象クラス
 */
class PixelWriter {
  public:
    PixelWriter(const FrameBufferConfig& config) : config_{config} {}

  // destructor
  virtual ~PixelWriter() = default;
  // 純粋仮想関数であるため、=0を指定
  virtual void Write(int x, int y, const PixelColor& c) = 0;

  protected:
    uint8_t* PixelAt(int x, int y) {
      return config_.frame_buffer + 4 * (config_.pixels_per_scan_line * y + x);
    }

  private:
    const FrameBufferConfig& config_; 
};

class RGBResv8BitPerColorPixelWriter : public PixelWriter {
  public:
    using PixelWriter::PixelWriter;

    // コンパイラにoverride修飾子をちゃんと伝える
    virtual void Write(int x, int y, const PixelColor& c) override {
      auto p = PixelAt(x, y);
      p[0] = c.r;
      p[1] = c.g;
      p[2] = c.b;
    }
};

class BGRResv8BitPerColorPixelWriter : public PixelWriter {
  public:
    using PixelWriter::PixelWriter;

    virtual void Write(int x, int y, const PixelColor& c) override {
      auto p = PixelAt(x, y);
      p[0] = c.b;
      p[1] = c.g;
      p[2] = c.r;
    }
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
// sizeはコンパイラから自動で渡されるらしい？？ [TODO]
void* operator new(size_t size, void* buf) {
  return buf;
}

// destructorで呼ばれるので便宜上定義しとく
void operator delete(void* obj) noexcept {}

char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter* pixel_writer;

extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config) {
  switch (frame_buffer_config.pixel_format) {
    case kPixelRGBResv8BitPerColor:
      pixel_writer = new(pixel_writer_buf)
        RGBResv8BitPerColorPixelWriter{frame_buffer_config};
      break;
    case kPixelBGRResv8BitPerColor:
      pixel_writer = new(pixel_writer_buf)
        BGRResv8BitPerColorPixelWriter{frame_buffer_config};
      break;
  }

  for (int x = 0; x < frame_buffer_config.horizontal_resolution; x++) {
    for (int y = 0; y < frame_buffer_config.vertical_resolution; y++) {
      // 一旦白塗り
      pixel_writer->Write(x, y, {255, 255, 255});
    }
  }
  for (int x = 0; x < 200; x++) {
    for (int y = 0; y < 100; y++) {
      // 緑の四角を描画
      pixel_writer->Write(x, y, {0, 255, 0});
    }
  }
  
  WriteAscii(*pixel_writer, 50, 50, 'A', {0, 0, 0});
  WriteAscii(*pixel_writer, 58, 50, 'A', {0, 0, 0});
  
  while (1) __asm__("hlt");
}