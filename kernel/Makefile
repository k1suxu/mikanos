# CXX = clang++
# LD = ld.lld

# --- ツール ---
ROOT_DIR := $(shell pwd)
CXX = $(ROOT_DIR)/../kernel_builder/clang+llvm-7.1.0-x86_64-linux-gnu-ubuntu-14.04/bin/clang++
CC  = $(ROOT_DIR)/../kernel_builder/clang+llvm-7.1.0-x86_64-linux-gnu-ubuntu-14.04/bin/clang
LD  = $(ROOT_DIR)/../kernel_builder/clang+llvm-7.1.0-x86_64-linux-gnu-ubuntu-14.04/bin/ld.lld

# --- コンパイルフラグ ---
BASEDIR="$(HOME)/osbook/devenv/x86_64-elf"
EDK2DIR="$(HOME)/edk2"

CFLAGS   = -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti -std=c++17
CPPFLAGS = -I$(BASEDIR)/include/c++/v1 -I$(BASEDIR)/include \
           -I$(BASEDIR)/include/freetype2 -I$(EDK2DIR)/MdePkg/Include \
           -I$(EDK2DIR)/MdePkg/Include/X64 -nostdlibinc -D__ELF__ \
           -D_LDBL_EQ_DBL -D_GNU_SOURCE -D_POSIX_TIMERS \
           -DEFIAPI='__attribute__((ms_abi))'

LDFLAGS = -L$(BASEDIR)/lib --entry KernelMain -z norelro --image-base 0x100000 --static

# --- ソースファイル ---
CPP = $(shell find . -name "*.cpp")
C   = $(shell find . -name "*.c")
ASSEMBLY = $(shell find . -name "*.asm")
OBJ = $(CPP:.cpp=.o) $(C:.c=.o) $(ASSEMBLY:.asm=.o)

# --- フォント ---
FONT_TXT = hankaku.txt
FONT_BIN = hankaku.bin
FONT_OBJ = hankaku.o

# --- ターゲット ---
TARGET = kernel.elf

# --- ビルド ---
all: $(TARGET)

$(TARGET): $(OBJ) $(FONT_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^ -lc -lc++

# C++ コンパイル
%.o: %.cpp Makefile
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

# C コンパイル
%.o: %.c Makefile
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# assembly to objectfile
%.o: %.asm Makefile
	nasm -f elf64 -o $@ $<

# フォント生成
$(FONT_BIN): $(FONT_TXT)
	../tools/makefont.py -o $@ $<

$(FONT_OBJ): $(FONT_BIN)
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 $< $@

# --- 依存関係 ---
.%.d: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MM $< > $@
	$(eval OBJ = $(<:.cpp=.o))
	sed --in-place 's|$(notdir $(OBJ))|$(OBJ)|' $@

.%.d: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $< > $@
	$(eval OBJ = $(<:.c=.o))
	sed --in-place 's|$(notdir $(OBJ))|$(OBJ)|' $@

.%.d: %.bin
	touch $@

DEPENDS = $(join $(dir $(OBJ)),$(addprefix .,$(notdir $(OBJ:.o=.d))))

.PHONY: depends
depends:
	$(MAKE) $(DEPENDS)

-include $(DEPENDS)

# --- クリーン ---
clean:
	rm -f $(OBJ) $(TARGET) $(FONT_OBJ) $(FONT_BIN) *.d

.PHONY: all clean depends
