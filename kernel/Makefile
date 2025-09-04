# CXX = clang++
# LD = ld.lld

# kernel/直下にclang, lldがあると想定
ROOT_DIR := $(shell pwd)
CXX = $(ROOT_DIR)/../kernel_builder/clang+llvm-7.1.0-x86_64-linux-gnu-ubuntu-14.04/bin/clang++
LD = $(ROOT_DIR)/../kernel_builder/clang+llvm-7.1.0-x86_64-linux-gnu-ubuntu-14.04/bin/ld.lld

CXXFLAGS = -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone \
					 -fno-exceptions -fno-rtti -std=c++17
# p.84, to set include path
BASEDIR="$(HOME)/osbook/devenv/x86_64-elf"
EDK2DIR="$(HOME)/edk2"
OPTIONAL_CPP_FLAGS = -I$(BASEDIR)/include/c++/v1 -I$(BASEDIR)/include \
										 -I$(BASEDIR)/include/freetype2 -I$(EDK2DIR)/MdePkg/Include \
										 -I$(EDK2DIR)/MdePkg/Include/X64 -nostdlibinc -D__ELF__ \
										 -D_LDBL_EQ_DBL -D_GNU_SOURCE -D_POSIX_TIMERS \
										 -DEFIAPI='__attribute__((ms_abi))'
LDFLAGS=-L$(BASEDIR)/lib

SRC = main.cpp
OBJ = $(SRC:.cpp=.o)
OUTPUT = kernel.elf
ENTRY = KernelMain
all: $(OUTPUT)

$(OUTPUT): $(OBJ)
	$(LD) $(LDFLAGS) --entry $(ENTRY) -z norelro --image-base 0x100000 --static -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(OPTIONAL_CPP_FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(OUTPUT)

.PHONY: all clean