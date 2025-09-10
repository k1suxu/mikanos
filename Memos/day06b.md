# ScanAllBus()について
ホストブリッジがmulti-functionデバイスである場合に、`for (uint8_t function = 1; ...)`となっているが、これは`for (uint8_t function = 0; ...)`が正しいはず(06cのブランチでは修正されている)

# 配置new
- [TODO] 配置newが消されている 