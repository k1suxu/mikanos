#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/BlockIo.h>
#include <Guid/FileInfo.h>

// ファイルディスクリプタは、
// - Type          (メモリ領域の種別)
// - PhysicalStart (物理メモリアドレスの先頭)
// - VirtualStart  (仮想メモリアドレスの先頭)
// - numberOfPages (メモリ領域の大きさ(4KiB単位))
// - Attribute     (使用可能用途)

struct MemoryMap {
  UINTN buffer_size;          // 書き込み先バッファ全体のサイズ
  VOID* buffer;               // メモリマップ本体(ファイルディスクリプタの配列)
  UINTN map_size;             // メモリマップのサイズ
  UINTN map_key;              // メモリマップ識別値(変化なし⇒メモリマップは変化していない)
  UINTN descriptor_size;      // ファイルディスクリプタのサイズ
  UINT32 descriptor_version;  // ディスクリプタのバージョン
};

EFI_STATUS GetMemoryMap(struct MemoryMap* map) {
  if (map->buffer == NULL) {
    return EFI_BUFFER_TOO_SMALL;
  }

  map->map_size = map->buffer_size;
  return gBS->GetMemoryMap(
      &map->map_size,
      (EFI_MEMORY_DESCRIPTOR*)map->buffer,
      &map->map_key,
      &map->descriptor_size,
      &map->descriptor_version);
}

const CHAR16* GetMemoryTypeUnicode(EFI_MEMORY_TYPE type) {
  switch (type) {
    case EfiReservedMemoryType: return L"EfiReservedMemoryType";
    case EfiLoaderCode: return L"EfiLoaderCode";
    case EfiLoaderData: return L"EfiLoaderData";
    case EfiBootServicesCode: return L"EfiBootServicesCode";
    case EfiBootServicesData: return L"EfiBootServicesData";
    case EfiRuntimeServicesCode: return L"EfiRuntimeServicesCode";
    case EfiRuntimeServicesData: return L"EfiRuntimeServicesData";
    case EfiConventionalMemory: return L"EfiConventionalMemory";
    case EfiUnusableMemory: return L"EfiUnusableMemory";
    case EfiACPIReclaimMemory: return L"EfiACPIReclaimMemory";
    case EfiACPIMemoryNVS: return L"EfiACPIMemoryNVS";
    case EfiMemoryMappedIO: return L"EfiMemoryMappedIO";
    case EfiMemoryMappedIOPortSpace: return L"EfiMemoryMappedIOPortSpace";
    case EfiPalCode: return L"EfiPalCode";
    case EfiPersistentMemory: return L"EfiPersistentMemory";
    case EfiMaxMemoryType: return L"EfiMaxMemoryType";
    default: return L"InvalidMemoryType";
  }
}

EFI_STATUS SaveMemoryMap(struct MemoryMap* map, EFI_FILE_PROTOCOL* file) {
  CHAR8 buf[256];
  UINTN len;

  CHAR8* header =
    "Index, Type, Type(name), PhysicalStart, NumberOfPages, Attribute\n";
  len = AsciiStrLen(header);
  file->Write(file, &len, header);

  Print(L"map->buffer = %08lx, map->map_size = %08lx\n",
      map->buffer, map->map_size);

  EFI_PHYSICAL_ADDRESS iter;
  int i;
  for (iter = (EFI_PHYSICAL_ADDRESS)map->buffer, i = 0;
       iter < (EFI_PHYSICAL_ADDRESS)map->buffer + map->map_size;
       iter += map->descriptor_size, i++) {
    EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)iter;
    len = AsciiSPrint(
        buf, sizeof(buf),
        "%u, %x, %-ls, %08lx, %lx, %lx\n",
        i, desc->Type, GetMemoryTypeUnicode(desc->Type),
        desc->PhysicalStart, desc->NumberOfPages,
        desc->Attribute & 0xffffflu);
    file->Write(file, &len, buf);
  }

  return EFI_SUCCESS;
}

EFI_STATUS OpenRootDir(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL** root) {
  EFI_LOADED_IMAGE_PROTOCOL* loaded_image;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;

  gBS->OpenProtocol(
      image_handle,
      &gEfiLoadedImageProtocolGuid,
      (VOID**)&loaded_image,
      image_handle,
      NULL,
      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

  gBS->OpenProtocol(
      loaded_image->DeviceHandle,
      &gEfiSimpleFileSystemProtocolGuid,
      (VOID**)&fs,
      image_handle,
      NULL,
      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

  fs->OpenVolume(fs, root);

  return EFI_SUCCESS;
}

// EFIAPI: 空文字列(プログラマ用の文字列)
EFI_STATUS EFIAPI UefiMain(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE* system_table) {
  Print(L"Hello, Mikan World!\n");

  CHAR8 memmap_buf[4096 * 4];
  struct MemoryMap memmap = {sizeof(memmap_buf), memmap_buf, 0, 0, 0, 0};
  GetMemoryMap(&memmap);

  EFI_FILE_PROTOCOL* root_dir;
  OpenRootDir(image_handle, &root_dir);

  EFI_FILE_PROTOCOL* memmap_file;
  // RW権限でO_CREATE
  root_dir->Open(
      root_dir, &memmap_file, L"\\memmap",
      EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);

  SaveMemoryMap(&memmap, memmap_file);
  memmap_file->Close(memmap_file);


  // ファイルopenの流れは、
  // 1. ブートローダでカーネルファイルを読み込み (Open)
  // 2. ファイル全体を確保できるメモリ領域の確保 (GetInfo, AllocatePages)
  // 3. ファイル内容をメモリに書き出す
  EFI_FILE_PROTOCOL* kernel_file;
  // kernel.eflをreadモードでopenする
  root_dir->Open(
      root_dir, &kernel_file, L"\\kernel.elf",
      EFI_FILE_MODE_READ, 0);

  UINTN file_info_size = sizeof(EFI_FILE_INFO) + sizeof(CHAR16) * 12;
  UINT8 file_info_buffer[file_info_size]; // file_info_sizeバイト(*8ビット)の領域を確保

  // kernel_fileの情報を取得。この際、file_info_sizeも実際の値に変化される
  // GetInfo(ファイル, , ファイル情報サイズ(実際のものに変更される), ファイル情報格納バッファ)
  kernel_file->GetInfo(
      kernel_file, &gEfiFileInfoGuid,
      &file_info_size, file_info_buffer);
  
  EFI_FILE_INFO* file_info = (EFI_FILE_INFO*)file_info_buffer;
  UINTN kernel_file_size = file_info->FileSize;

  EFI_PHYSICAL_ADDRESS kernel_base_addr = 0x100000;
  gBS->AllocatePages(
      AllocateAddress, EfiLoaderData,
      (kernel_file_size + 0xfff) / 0x1000, &kernel_base_addr);
  kernel_file->Read(kernel_file, &kernel_file_size, (VOID*)kernel_base_addr);
  Print(L"Kernel: 0x%0lx (%lu bytes)\n", kernel_base_addr, kernel_file_size);

  EFI_STATUS status;
  status = gBS->ExitBootServices(image_handle, memmap.map_key);
  if (EFI_ERROR(status)) {
    status = GetMemoryMap(&memmap);
    if (EFI_ERROR(status)) {
      // boot-serviceが停止しないとカーネルが動かない(OSは動かせない)ので、無限ループさせとく
      Print(L"failed to get memory map: %r\n", status); 
      while (1);
    }
    status = gBS->ExitBootServices(image_handle, memmap.map_key);
    if (EFI_ERROR(status)) {
      Print(L"Could not exit boot service: %r\n", status); 
      while (1);
    }
  }
  
  UINT64 entry_addr = *(UINT64*)(kernel_base_addr + 24); // ELFファイルのエントリポイントアドレスは24-32バイト目に書かれてる

  typedef void EntryPointType(void); // void(void)関数の型をtypedefする
  EntryPointType* entry_point = (EntryPointType*)entry_addr;
  entry_point();
  
  // ここの処理は呼ばれないはず(kernelの方で無限ループさせる)
  Print(L"All done\n");

  while (1);
  return EFI_SUCCESS;
}
