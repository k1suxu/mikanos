#pragma once

#include <stdint.h>

extern "C" {
  // assemblyで直書きしてるので、関数名の名前修飾は起きない
  void IoOut32(uint16_t addr, uint32_t data);
  uint32_t IoIn32(uint16_t addr);
}