#include <new>
#include <cerrno>

// スタブ実装

// newでメモリ確保に失敗した際に実行すべき関数を返すget_new_handlerを用意する必要がある
// (スタブなので適当にnullptrを返しているだけ。newのfailは許されない？)
std::new_handler std::get_new_handler() noexcept {
  return nullptr;
}

// メモリ確保は一律エラーとする
extern "C" int posix_memalign(void**, size_t, size_t) {
  return ENOMEM;
}