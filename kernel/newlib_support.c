#include <errno.h>
#include <sys/types.h>

// osbook/devenv/x86_64-elf/includeをcpp-propertiesのincludePathに追加する必要がある(intellisense用)
// sbrk : start break => ヒープ領域拡張. 今は常にエラーとする((caddr_t)-1を返すのが習慣らしい？)
caddr_t sbrk(int incr) {
  errno = ENOMEM;
  return (caddr_t) - 1;
}
