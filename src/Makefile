BUILD:=../build
SRC:=.

MULTIBOOT2:=0x10000
ENTRYPOINT:=$(shell python -c "print(f'0x{$(MULTIBOOT2) + 64:x}')") # MULTIBOOT2头必须以64字节对齐

CFLAGS:= -m32 # 32位程序
CFLAGS+= -fno-builtin # 不需要gcc内置函数,例如内置的memcpy
CFLAGS+= -nostdinc    # 不需要标准头文件,只能用自己的函数
CFLAGS+= -fno-pic     # 不需要位置无关的代码 position independent code
CFLAGS+= -fno-pie     # 不需要位置无关的可执行程序 position independent executable
CFLAGS+= -nostdlib     # 不需要标准库
CFLAGS+= -fno-stack-protector # 不需要栈保护
CFLAGS+= -DNEONIX # 定义 NEONIX
CFLAGS:=$(strip $(CFLAGS))  # 删除头尾不必要的空格和tab

LDFLAGS:= -m elf_i386 \
		-static \
		-Ttext $(ENTRYPOINT)\
		--section-start=.multiboot2=$(MULTIBOOT2)
LDFLAGS:=$(strip ${LDFLAGS})

DEBUG:= -g
INCLUDE:=-I$(SRC)/include

# 先匹配精确的,再匹配模糊的
# 静态链接，$^表示所有依赖文件
$(BUILD)/boot/%.bin: $(SRC)/boot/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f bin $< -o $@

$(BUILD)/%.o: $(SRC)/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f elf32 $(DEBUG) $< -o $@

$(BUILD)/%.o: $(SRC)/%.c
	$(shell mkdir -p $(dir $@))
	gcc $(CFLAGS) $(DEBUG) $(INCLUDE) -c $< -o $@

$(BUILD)/lib/libc.o: $(BUILD)/lib/crt.o \
	$(BUILD)/lib/crt1.o \
	$(BUILD)/lib/string.o \
	$(BUILD)/lib/vsprintf.o \
	$(BUILD)/lib/stdlib.o \
	$(BUILD)/lib/syscall.o \
	$(BUILD)/lib/printf.o \
	$(BUILD)/lib/assert.o \
	$(BUILD)/lib/time.o \

	ld -m elf_i386 -r $^ -o $@

BUILTIN_APPS := \
	$(BUILD)/builtin/init.out \
	$(BUILD)/builtin/neosh.out \
	$(BUILD)/builtin/env.out \
	$(BUILD)/builtin/echo.out \
	$(BUILD)/builtin/cat.out \
	$(BUILD)/builtin/ls.out \
	$(BUILD)/builtin/dup.out \

$(BUILD)/builtin/%.out: $(BUILD)/builtin/%.o \
	$(BUILD)/lib/libc.o \

	ld -m elf_i386 -static $^ -o $@ -Ttext 0x1001000

# 注意，这里的文件是有依赖关系的，builtin必须放在最后面链接
$(BUILD)/kernel.bin: \
	$(BUILD)/kernel/start.o \
	$(BUILD)/kernel/main.o \
	$(BUILD)/kernel/arena.o \
	$(BUILD)/kernel/assert.o \
	$(BUILD)/kernel/buffer.o \
	$(BUILD)/kernel/clock.o \
	$(BUILD)/kernel/device.o \
	$(BUILD)/kernel/debug.o \
	$(BUILD)/kernel/execve.o \
	$(BUILD)/kernel/console.o \
	$(BUILD)/kernel/gate.o \
	$(BUILD)/kernel/global.o \
	$(BUILD)/kernel/handler.o \
	$(BUILD)/kernel/ide.o \
	$(BUILD)/kernel/interrupt.o \
	$(BUILD)/kernel/io.o \
	$(BUILD)/kernel/keyboard.o \
	$(BUILD)/kernel/memory.o \
	$(BUILD)/kernel/mutex.o \
	$(BUILD)/kernel/printk.o \
	$(BUILD)/kernel/ramdisk.o \
	$(BUILD)/kernel/rtc.o \
	$(BUILD)/kernel/schedule.o \
	$(BUILD)/kernel/serial.o \
	$(BUILD)/kernel/system.o \
	$(BUILD)/kernel/task.o \
	$(BUILD)/kernel/thread.o \
	$(BUILD)/kernel/time.o \
	$(BUILD)/fs/bmap.o \
	$(BUILD)/fs/dev.o \
	$(BUILD)/fs/file.o \
	$(BUILD)/fs/inode.o \
	$(BUILD)/fs/namei.o \
	$(BUILD)/fs/pipe.o \
	$(BUILD)/fs/stat.o \
	$(BUILD)/fs/super.o \
	$(BUILD)/lib/bitmap.o \
	$(BUILD)/lib/fifo.o \
	$(BUILD)/lib/list.o \
	$(BUILD)/lib/printf.o \
	$(BUILD)/lib/stdlib.o \
	$(BUILD)/lib/string.o \
	$(BUILD)/lib/syscall.o \
	$(BUILD)/lib/vsprintf.o \
	
	$(shell mkdir -p $(dir $@))
	ld ${LDFLAGS} $^ -o $@

$(BUILD)/system.bin: $(BUILD)/kernel.bin
	objcopy -O binary $< $@

$(BUILD)/system.map: $(BUILD)/kernel.bin
	nm $< | sort > $@

include utils/image.mk
include utils/cdrom.mk
include utils/cmd.mk

.PHONY: clean
clean:
	rm -rf $(BUILD)