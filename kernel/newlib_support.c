#include <errno.h>
#include <sys/types.h>

void _exit(void) {
  while (1) __asm__("hlt");
}

// osbook/devenv/x86_64-elf/includeをcpp-propertiesのincludePathに追加する必要がある(intellisense用)
// sbrk : start break => ヒープ領域拡張. 今は常にエラーとする((caddr_t)-1を返すのが習慣らしい？)
caddr_t sbrk(int incr) {
  errno = ENOMEM;
  return (caddr_t) - 1;
}

int getpid(void) {
  return 1;
}

int kill(int pid, int sig) {
  errno = EINVAL;
  return -1;
}