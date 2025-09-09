#include <cstdint>
#include <cstddef>
#include <cstdio>

#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "font.hpp"
#include "console.hpp"

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
  
  Console console{*pixel_writer, {0, 0, 0}, {255, 255, 255}};

  char buf[128];
  for (int i = 0; i < 27; i++) {
    sprintf(buf, "line %d\n", i);
    console.PutString(buf);
  }
  
  while (1) __asm__("hlt");
}