# Kernelに入れないバグが発生
- cf. https://github.com/uchan-nos/os-from-zero/issues/134
- どうやら、リンカとしてlld14を使うとファイルロードの際のエントリポイントアドレスのオフセットがずれるっぽい？？
- lld7を使うと解決するとのことなので、試してみる⇒いけた (cf. https://github.com/uchan-nos/mikanos-build/issues/35)

- WIP: また、day04bのブートローダまでの実装が終わるとlld14にリンカをupdateしても動くらしい