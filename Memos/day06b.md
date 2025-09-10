# ScanAllBus()について
ホストブリッジがmulti-functionデバイスである場合に、`for (uint8_t function = 1; ...)`となっているが、これは`for (uint8_t function = 0; ...)`が正しいはず(06cのブランチでは修正されている)

# 配置new
- [TODO] 配置newが消されている
  - おそらく、`pci.hpp`にて、`#include<array>`されているためにlibcの方の配置newが宣言されている(`#include<new>`が起こっている？)
  - intellisenseがうまく働かなかったので無理やりincludepath設定に`osbook/devenv/x86_64-elf/include/c++/v1`を追加した