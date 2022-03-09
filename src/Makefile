BUILD:=../build
SRC:=.

ENTRYPOINT:=0x10000

CFLAGS:= -m32 # 32位程序
CFLAGS+= -fno-builtin # 不需要gcc内置函数,例如内置的memcpy
CFLAGS+= -nostdinc    # 不需要标准头文件,只能用自己的函数
CFLAGS+= -fno-pic     # 不需要无关的代码position independent code
CFLAGS+= -fno-pie     # position independent executable
CFLAGS+= -nostdlib     # 不需要标准库
CFLAGS+= -fno-stack-protector # 不需要栈保护
CFLAGS:=$(strip $(CFLAGS))


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

$(BUILD)/kernel.bin: \
	$(BUILD)/kernel/start.o \
	$(BUILD)/kernel/main.o \
	$(BUILD)/kernel/assert.o \
	$(BUILD)/kernel/clock.o \
	$(BUILD)/kernel/debug.o \
	$(BUILD)/kernel/console.o \
	$(BUILD)/kernel/global.o \
	$(BUILD)/kernel/handler.o \
	$(BUILD)/kernel/interrupt.o \
	$(BUILD)/kernel/io.o \
	$(BUILD)/kernel/printk.o \
	$(BUILD)/kernel/schedule.o \
	$(BUILD)/kernel/task.o \
	$(BUILD)/kernel/time.o \
	$(BUILD)/lib/stdlib.o \
	$(BUILD)/lib/string.o \
	$(BUILD)/lib/vsprintf.o \
	
	$(shell mkdir -p $(dir $@))
	ld -m elf_i386 -static $^ -o $@ -Ttext $(ENTRYPOINT)

$(BUILD)/system.bin: $(BUILD)/kernel.bin
	objcopy -O binary $< $@

$(BUILD)/system.map: $(BUILD)/kernel.bin
	nm $< | sort > $@

# 镜像大小16M,boot程序512B,loader程序从第2个扇区开始4*512=2KB
$(BUILD)/master.img: $(BUILD)/boot/boot.bin \
	$(BUILD)/boot/loader.bin \
	$(BUILD)/system.bin \
	$(BUILD)/system.map \

	yes | bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@
	dd if=$(BUILD)/boot/boot.bin of=$@ bs=512 count=1 conv=notrunc
	dd if=$(BUILD)/boot/loader.bin of=$@ bs=512 count=4 seek=2 conv=notrunc
	dd if=$(BUILD)/system.bin of=$@ bs=512 count=200 seek=10 conv=notrunc

# 写入u盘
.PHONY: usb
usb: boot.bin /dev/sdb
	sudo dd if=/dev/sdb/ of=tmp.bin bs=512 count=1 conv=notrunc
	cp tmp.bin usb.bin
	sudo rm tmp.bin
	dd if=boot.bin of=usb.bin bs=446 count=1 conv=notrunc
	sudo dd if=usb.bin of=/dev/sdb bs=512 count=1 conv=notrunc
	rm usb.bin

test: $(BUILD)/master.img

.PHONY: clean
clean:
	rm -rf $(BUILD)

.PHONY: bochs
bochs: $(BUILD)/master.img
	bochs -q -f ../bochs/bochsrc

.PHONY: qemu
qemu: $(BUILD)/master.img
	qemu-system-i386 \
		-m 32M \
		-boot c \
		-drive file=$<,if=ide,index=0,media=disk,format=raw \
		-audiodev pa,id=hda \
		-machine pcspk-audiodev=hda \

# -s表示监听1234端口,-S表示gdb在没有连接之前不执行
.PHONY: qemug
qemug: $(BUILD)/master.img
	qemu-system-i386 \
		-s -S \
		-m 32M \
		-boot c \
		-drive file=$<,index=0,media=disk,format=raw \
		-audiodev pa,id=hda \
		-machine pcspk-audiodev=hda \
