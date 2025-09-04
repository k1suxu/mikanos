/**
 * @file main.cpp
 * 
 * カーネル本体のプログラムを書いたファイル.
 */

#include <cstdint>
#include <cstddef>

#include "frame_buffer_config.hpp"

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
  while (1) __asm__("hlt");
}