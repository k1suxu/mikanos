# エラー (2025/08/28)
> /.../edk2/MikanLoaderPkg/MikanLoaderPkg.dsc(...): error 4000: Instance of library class [RegisterFilterLib] is not found
        in [/.../edk2/MdePkg/Library/BaseLib/BaseLib.inf] [X64]
        consumed by module [/.../edk2/MikanLoaderPkg/Loader.inf]

どうやら、edkのupdateによってRegisterFilterLibが必須となっているよう。

cf. [これ](https://github.com/uchan-nos/mikanos/commit/b5f7740c04002e67a95af16a5c6e073b664bf3f5)の`MikanLoaderPkg.dsc`へのパッチを適用するとうまくいった