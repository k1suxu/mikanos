# CXX = clang++
# LD = ld.lld

# kernel/直下にclang, lldがあると想定
ROOT_DIR := $(shell pwd)
CXX = $(ROOT_DIR)/clang+llvm-7.1.0-x86_64-linux-gnu-ubuntu-14.04/bin/clang++
LD = $(ROOT_DIR)/clang+llvm-7.1.0-x86_64-linux-gnu-ubuntu-14.04/bin/ld.lld

CXXFLAGS = -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone \
			-fno-exceptions -fno-rtti -std=c++17

SRC = main.cpp
OBJ = $(SRC:.cpp=.o)
OUTPUT = kernel.elf
ENTRY = KernelMain
all: $(OUTPUT)

$(OUTPUT): $(OBJ)
	$(LD) --entry $(ENTRY) -z norelro --image-base 0x100000 --static -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(OUTPUT)

.PHONY: all clean