#pragma once

#include <cstdint>
#include <array>

#include "error.hpp"

namespace pci {
  const uint16_t kConfigAddress = 0x0cf8; // CONFIG_ADDRESSレジスタのIOポート上のアドレス
  const uint16_t kConfigData = 0x0cfc;    // CONFIG_DATAレジスタのIOポート上のアドレス

  // PCI Class Code
  struct ClassCode {
    uint8_t base, sub, interface;

    bool Match(uint8_t b) { return b == base; }
    bool Match(uint8_t b, uint8_t s) { return Match(b) && s == sub; }
    bool Match(uint8_t b, uint8_t s, uint8_t i) {
      return Match(b, s) && i == interface;
    }
  };

  struct Device {
    uint8_t bus, device, function, header_type;
    ClassCode class_code;
  };

  void WriteAddress(uint32_t address); // CONFIG_ADDRESSに指定アドレスを書き込む
  void WriteData(uint32_t value);      // CONFIG_DATAに指定データを書き込む
  uint32_t ReadData();                 // CONFIG_DATA上のレジスタ値を読み込み

  uint16_t ReadVendorId(uint8_t bus, uint8_t device, uint8_t function);   // ベンダ ID レジスタの読み取り(PCIコンフィギュレーション空間のヘッダタイプ非依存)
  uint16_t ReadDeviceId(uint8_t bus, uint8_t device, uint8_t function);   // デバイス ID レジスタの読み取り(PCIコンフィギュレーション空間のヘッダタイプ非依存)
  uint8_t ReadHeaderType(uint8_t bus, uint8_t device, uint8_t function); // ヘッダタイプレジスタの読み取り(PCIコンフィギュレーション空間のヘッダタイプ非依存)
  /**
   * 旧仕様：
   * - 31:24 : ベースクラス
   * - 23:16 : サブクラス
   * - 15:8  : インターフェース
   * - 7:0   : リビジョンID
   * 
   * としていたものを、Class Codeクラスとして使いやすく変更
   */
  ClassCode ReadClassCode(uint8_t bus, uint8_t device, uint8_t function);  // クラスコードレジスタの読み取り(PCIコンフィギュレーション空間のヘッダタイプ非依存)

  inline uint16_t ReadVendorId(const Device& dev) {
    return ReadVendorId(dev.bus, dev.device, dev.function);
  }

  // I/O Port を用いてPCIデバイスの 32 ビットレジスタを読み取る
  uint32_t ReadConfReg(const Device& dev, uint8_t reg_addr);
  void WriteConfReg(const Device& dev, uint8_t reg_addr, uint32_t value);

  /**
   * - 23:16 : サブオーディネイトバス番号
   * - 15:8  : セカンダリバス番号
   * - 7:0   : リビジョン番号
   */
  uint32_t ReadBusNumbers(uint8_t bus, uint8_t device, uint8_t function); // バス番号読み取り(ヘッダタイプ1を過程)

  bool IsSingleFunctionDevice(uint8_t header_type);

  inline std::array<Device, 32> devices;
  inline int num_device;

  Error ScanAllBus();

  constexpr uint8_t CalcBarAddress(unsigned int bar_index) {
    // cf. PCI-Configuration-Space Structure
    return 0x10 + 4 * bar_index;
  }

  // PCIコンフィギュレーション空間からBAR0-BAR5のうち指定されたBARと後続のBARを読み64ビットにまとめて返す
  WithError<uint64_t> ReadBar(Device& device, unsigned int bar_index);
}