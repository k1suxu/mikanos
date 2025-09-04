#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/BlockIo.h>
#include <Guid/FileInfo.h>
#include "frame_buffer_config.hpp"
#include "elf.hpp"

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
  EFI_STATUS status;
  CHAR8 buf[256];
  UINTN len;

  CHAR8* header =
    "Index, Type, Type(name), PhysicalStart, NumberOfPages, Attribute\n";
  len = AsciiStrLen(header);
  status = file->Write(file, &len, header);
  if (EFI_ERROR(status)) {
    return status;
  }

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

    status = file->Write(file, &len, buf);
    if (EFI_ERROR(status)) {
      return status;
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS OpenRootDir(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL** root) {
  EFI_STATUS status;
  EFI_LOADED_IMAGE_PROTOCOL* loaded_image;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;

  status = gBS->OpenProtocol(
    image_handle,
    &gEfiLoadedImageProtocolGuid,
    (VOID**)&loaded_image,
    image_handle,
    NULL,
    EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  if (EFI_ERROR(status)) {
    return status;
  }

  status = gBS->OpenProtocol(
    loaded_image->DeviceHandle,
    &gEfiSimpleFileSystemProtocolGuid,
    (VOID**)&fs,
    image_handle,
    NULL,
    EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  if (EFI_ERROR(status)) {
    return status;
  }

  return fs->OpenVolume(fs, root);
}

// UEFI の GOP (Graphics Output Protocol) を利用して描画情報を取得
EFI_STATUS OpenGOP(EFI_HANDLE image_handle,
                   EFI_GRAPHICS_OUTPUT_PROTOCOL** gop) {
  EFI_STATUS status;
  UINTN num_gop_handles = 0;
  EFI_HANDLE* gop_handles = NULL;
  status = gBS->LocateHandleBuffer(
    ByProtocol,
    &gEfiGraphicsOutputProtocolGuid,
    NULL,
    &num_gop_handles,
    &gop_handles);
  if (EFI_ERROR(status)) {
    return status;
  }
  
  status = gBS->OpenProtocol(
    gop_handles[0],
    &gEfiGraphicsOutputProtocolGuid,
    (VOID**)gop,
    image_handle,
    NULL,
    EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  if (EFI_ERROR(status)) {
    return status;
  }

  FreePool(gop_handles);

  return EFI_SUCCESS;
}

const CHAR16* GetPixelFormatUnicode(EFI_GRAPHICS_PIXEL_FORMAT fmt) {
  switch (fmt) {
    case PixelRedGreenBlueReserved8BitPerColor:
      return L"PixelRedGreenBlueReserved8BitPerColor";
    case PixelBlueGreenRedReserved8BitPerColor:
      return L"PixelBlueGreenRedReserved8BitPerColor";
    case PixelBitMask:
      return L"PixelBitMask";
    case PixelBltOnly:
      return L"PixelBltOnly";
    case PixelFormatMax:
      return L"PixelFormatMax";
    default:
      return L"InvalidPixelFormat";
  }
}

void Halt(void) {
  while (1) __asm__("hlt");
}

// Loaderにおいて、該当ファイルのメモリ範囲(virtual)を取得する
void CalcLoadAddressRange(Elf64_Ehdr* ehdr, UINT64* first, UINT64* last) {
  // プログラムヘッダの位置はファイルヘッダのe_phoffに格納されチエル
  Elf64_Phdr* phdr = (Elf64_Phdr*)((UINT64)ehdr + ehdr->e_phoff);
  *first = MAX_UINT64;
  *last = 0;
  // プログラムヘッダ内の各セグメントに対して(virtual) addressのrangeを求めて、chmin, chmaxする
  for (Elf64_Half i = 0; i < ehdr->e_phnum; i++) {
    if (phdr[i].p_type != PT_LOAD) continue;
    *first = MIN(*first, phdr[i].p_vaddr);
    *last = MAX(*last, phdr[i].p_vaddr + phdr[i].p_memsz);
  }
}

// Loadタイプのセグメントを対象の(virtual) addrへ格納
void CopyLoadSegments(Elf64_Ehdr* ehdr) {
  Elf64_Phdr* phdr = (Elf64_Phdr*)((UINT64)ehdr + ehdr->e_phoff);
  for (Elf64_Half i = 0; i < ehdr->e_phnum; i++) {
    if (phdr[i].p_type != PT_LOAD) continue;

    // ファイル内における該当セグメントの開始アドレスを求める
    UINT64 segm_in_file = (UINT64)ehdr + phdr[i].p_offset;
    // CopyMem(dst, src, len)
    CopyMem((VOID*)phdr[i].p_vaddr, (VOID*)segm_in_file, phdr[i].p_filesz);

    // memszとfileszの差分を0埋めする(.bss等)
    UINTN remain_bytes = phdr[i].p_memsz - phdr[i].p_filesz;
    SetMem((VOID*)(phdr[i].p_vaddr + phdr[i].p_filesz), remain_bytes, 0);
  }
}

// EFIAPI: 空文字列(プログラマ用の文字列)
EFI_STATUS EFIAPI UefiMain(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE* system_table) {
  EFI_STATUS status;


  /*** memory map ***/
  Print(L"Hello, Mikan World!\n");

  CHAR8 memmap_buf[4096 * 4];
  struct MemoryMap memmap = {sizeof(memmap_buf), memmap_buf, 0, 0, 0, 0};
  status = GetMemoryMap(&memmap);
  if (EFI_ERROR(status)) {
    Print(L"failed to get memory map: %r\n", status);
    Halt(); // 失敗した場合は無限halt
  }

  EFI_FILE_PROTOCOL* root_dir;
  status = OpenRootDir(image_handle, &root_dir);
  if (EFI_ERROR(status)) {
    Print(L"failed to open root directory: %r\n", status);
    Halt();
  }

  EFI_FILE_PROTOCOL* memmap_file;
  // RW権限でO_CREATE
  status = root_dir->Open(
    root_dir, &memmap_file, L"\\memmap",
    EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
  if (EFI_ERROR(status)) {
    Print(L"failed to open file '\\memmap': %r\n", status);
    Print(L"Ignored.\n");
  } else {
    status = SaveMemoryMap(&memmap, memmap_file);
    if (EFI_ERROR(status)) {
      Print(L"failed to save memory map: %r\n", status);
      Halt();
    }
    status = memmap_file->Close(memmap_file);
    if (EFI_ERROR(status)) {
      Print(L"failed to close memory map: %r\n", status);
      Halt();
    }
  }



  /*** get GOP data ***/
  // 本とは異なり、描画情報を白塗りつぶしの後に出力している
  EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
  status = OpenGOP(image_handle, &gop);
  if (EFI_ERROR(status)) {
    Print(L"failed to open GOP: %r\n", status);
    Halt();
  }

  UINT8* frame_buffer = (UINT8*)gop->Mode->FrameBufferBase;
  for (UINTN i = 0; i < gop->Mode->FrameBufferSize; i++) {
    frame_buffer[i] = 255; // 一旦は白で塗りつぶし。
  }

  Print(L"Resolution: %ux%u, Pixel Format: %s, %u pixels/line\n",
    gop->Mode->Info->HorizontalResolution,
    gop->Mode->Info->VerticalResolution,
    GetPixelFormatUnicode(gop->Mode->Info->PixelFormat),
    gop->Mode->Info->PixelsPerScanLine);
  Print(L"Frame Buffer: 0x%0lx - 0x%0lx, Size: %lu bytes\n",
    gop->Mode->FrameBufferBase,
    gop->Mode->FrameBufferBase + gop->Mode->FrameBufferSize,
    gop->Mode->FrameBufferSize);
  

  /*** read kernel and put on memory ***/
  // ファイルloaderの流れは、
  // 1. UEFI Openプロトコルを用いてカーネルファイルを読み込み (Open)
  // 2. ファイル情報(ファイル全体のサイズ)を取得
  // 3. ファイルサイズ分のPoolをAllocatePoolで確保
  // 4. 確保したPool領域にkernelファイル全体を読み込み、ヘッダ情報等を取得
  // 5. ヘッダ情報を元にLOADセグメントを書きたいメモリ領域にPoolからCopy
  // 6. 確保したPool領域の解放

  EFI_FILE_PROTOCOL* kernel_file; // これ自体はUEFIのFILE_PROTOCOLであってkernelファイルそのものではない
  // 1. kernel.eflをreadモードでopenする
  status = root_dir->Open(
    root_dir, &kernel_file, L"\\kernel.elf",
    EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR(status)) {
    Print(L"failed to open file '\\kernel.elf': %r\n", status);
    Halt();
  }

  UINTN file_info_size = sizeof(EFI_FILE_INFO) + sizeof(CHAR16) * 12;
  UINT8 file_info_buffer[file_info_size]; // file_info_sizeバイト(*8ビット)の領域を確保

  // 2. kernel_fileの情報を取得。この際、file_info_sizeも実際の値に変化される
  // GetInfo(ファイル, , ファイル情報サイズ(実際のものに変更される), ファイル情報格納バッファ)
  status = kernel_file->GetInfo(
    kernel_file, &gEfiFileInfoGuid,
    &file_info_size, file_info_buffer);
  if (EFI_ERROR(status)) {
    Print(L"failed to get file information: %r\n", status);
    Halt();
  }
  
  EFI_FILE_INFO* file_info = (EFI_FILE_INFO*)file_info_buffer;
  UINTN kernel_file_size = file_info->FileSize;

  // 3. Poolを確保
  VOID* kernel_buffer;
  status = gBS->AllocatePool(EfiLoaderData, kernel_file_size, &kernel_buffer);
  if (EFI_ERROR(status)) {
    Print(L"failed to allocate pool: %r\n", status);
    Halt();
  }
  // 4-a. UEFI Readプロトコルを用いて、ストレージからRead(読み出してPoolに書き込み)
  status = kernel_file->Read(kernel_file, &kernel_file_size, kernel_buffer);
  if (EFI_ERROR(status)) {
    Print(L"error: %r\n", status);
    Halt();
  }

  // 4-b. ファイルサイズの計算
  Elf64_Ehdr* kernel_ehdr = (Elf64_Ehdr*)kernel_buffer;
  UINT64 kernel_first_addr, kernel_last_addr;
  CalcLoadAddressRange(kernel_ehdr, &kernel_first_addr, &kernel_last_addr);
  // 5. 一次退避領域から実際に入れたいメモリ領域に移す
  UINTN num_pages = (kernel_last_addr - kernel_first_addr + 0xfff) / 0x1000;
  status = gBS->AllocatePages(AllocateAddress, EfiLoaderData,
                              num_pages, &kernel_first_addr);
  if (EFI_ERROR(status)) {
    Print(L"failed to allocate pages: %r\n", status);
    Halt();
  }

  // 6. Poolを解放
  CopyLoadSegments(kernel_ehdr);
  Print(L"Kernel: 0x%0lx - 0x%0lx\n", kernel_first_addr, kernel_last_addr);

  status = gBS->ExitBootServices(image_handle, memmap.map_key);
  if (EFI_ERROR(status)) {
    status = GetMemoryMap(&memmap);
    if (EFI_ERROR(status)) {
      // boot-serviceが停止しないとカーネルが動かない(OSは動かせない)ので、無限ループさせとく
      Print(L"failed to get memory map: %r\n", status);
      Halt();
    }
    status = gBS->ExitBootServices(image_handle, memmap.map_key);
    if (EFI_ERROR(status)) {
      Print(L"Could not exit boot service: %r\n", status);
      Halt();
    }
  }
  
  UINT64 entry_addr = *(UINT64*)(kernel_first_addr + 24); // ELFファイルのエントリポイントアドレスは24-32バイト目に書かれてる

  struct FrameBufferConfig config = {
    (UINT8*)gop->Mode->FrameBufferBase,
    gop->Mode->Info->PixelsPerScanLine,
    gop->Mode->Info->HorizontalResolution,
    gop->Mode->Info->VerticalResolution,
    0 // 後のswitch文で上書きされる
  };
  switch (gop->Mode->Info->PixelFormat) {
    case PixelRedGreenBlueReserved8BitPerColor:
      config.pixel_format = kPixelRGBResv8BitPerColor;
      break;
    case PixelBlueGreenRedReserved8BitPerColor:
      config.pixel_format = kPixelBGRResv8BitPerColor;
      break;
    default:
      Print(L"Unimplemented pixel format: %d\n", gop->Mode->Info->PixelFormat);
      Halt();
  }

  typedef void EntryPointType(const struct FrameBufferConfig*);
  EntryPointType* entry_point = (EntryPointType*)entry_addr;
  entry_point(&config);
  
  // ここの処理は呼ばれないはず(kernelの方で無限ループさせる)
  Print(L"All done\n");

  while (1);
  return EFI_SUCCESS;
}
