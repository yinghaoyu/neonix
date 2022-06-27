BUILD:=../build
SRC:=.

MULTIBOOT2:=0x10000
ENTRYPOINT:=0x10040 # MULTIBOOT2头必须以64字节对齐

CFLAGS:= -m32 # 32位程序
CFLAGS+= -fno-builtin # 不需要gcc内置函数,例如内置的memcpy
CFLAGS+= -nostdinc    # 不需要标准头文件,只能用自己的函数
CFLAGS+= -fno-pic     # 不需要无关的代码position independent code
CFLAGS+= -fno-pie     # position independent executable
CFLAGS+= -nostdlib     # 不需要标准库
CFLAGS+= -fno-stack-protector # 不需要栈保护
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

$(BUILD)/kernel.bin: \
	$(BUILD)/kernel/start.o \
	$(BUILD)/kernel/main.o \
	$(BUILD)/kernel/assert.o \
	$(BUILD)/kernel/clock.o \
	$(BUILD)/kernel/debug.o \
	$(BUILD)/kernel/console.o \
	$(BUILD)/kernel/gate.o \
	$(BUILD)/kernel/global.o \
	$(BUILD)/kernel/handler.o \
	$(BUILD)/kernel/interrupt.o \
	$(BUILD)/kernel/io.o \
	$(BUILD)/kernel/memory.o \
	$(BUILD)/kernel/mutex.o \
	$(BUILD)/kernel/printk.o \
	$(BUILD)/kernel/rtc.o \
	$(BUILD)/kernel/schedule.o \
	$(BUILD)/kernel/task.o \
	$(BUILD)/kernel/thread.o \
	$(BUILD)/kernel/time.o \
	$(BUILD)/lib/bitmap.o \
	$(BUILD)/lib/list.o \
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

# 镜像大小16M,boot程序512B,loader程序从第2个扇区开始4*512=2KB
$(BUILD)/master.img: $(BUILD)/boot/boot.bin \
	$(BUILD)/boot/loader.bin \
	$(BUILD)/system.bin \
	$(BUILD)/system.map \

# 创建一个16M的硬盘镜像
	yes | bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@

# 将boot.bin写入主引导扇区, notrunc表示非截断
	dd if=$(BUILD)/boot/boot.bin of=$@ bs=512 count=1 conv=notrunc

# 将loader.bin写入硬盘
	dd if=$(BUILD)/boot/loader.bin of=$@ bs=512 count=4 seek=2 conv=notrunc

# 测试system.bin小于100k, 否则需要修改下面的count
	test -n "$$(find $(BUILD)/system.bin -size -100k)"

# 将system.bin写入硬盘
	dd if=$(BUILD)/system.bin of=$@ bs=512 count=200 seek=10 conv=notrunc

$(BUILD)/kernel.iso : $(BUILD)/kernel.bin $(SRC)/utils/grub.cfg
# 检测内核文件是否合法
	grub-file --is-x86-multiboot2 $<
# 创建 iso 目录
	mkdir -p $(BUILD)/iso/boot/grub
# 拷贝内核文件
	cp $< $(BUILD)/iso/boot
# 拷贝 grub 配置文件
	cp $(SRC)/utils/grub.cfg $(BUILD)/iso/boot/grub
# 生成 iso 文件
	grub-mkrescue -o $@ $(BUILD)/iso

image: $(BUILD)/master.img

.PHONY: clean
clean:
	rm -rf $(BUILD)

.PHONY: bochs
bochs: $(BUILD)/master.img
	bochs -q -f ../bochs/bochsrc -unlock

.PHONY: bochsg
bochsg: $(BUILD)/master.img
	bochs-gdb -q -f ../bochs/bochsrc.gdb -unlock

.PHONY: bochsb
bochsb: $(BUILD)/kernel.iso
	bochs -q -f ../bochs/bochsrc.grub -unlock

QEMU:= qemu-system-i386 \
		-m 32M \
		-audiodev pa,id=hda \
		-machine pcspk-audiodev=hda \
		-rtc base=localtime \

QEMU_DISK:=-boot c \
		-drive file=$(BUILD)/master.img,if=ide,index=0,media=disk,format=raw \

QEMU_CDROM:=-boot d \
		-drive file=$(BUILD)/kernel.iso,media=cdrom \

QEMU_DEBUG:= -s -S

.PHONY: qemu
qemu: $(BUILD)/master.img
	$(QEMU) $(QEMU_DISK)

# -s表示监听1234端口,-S表示gdb在没有连接之前不执行
.PHONY: qemug
qemug: $(BUILD)/master.img
	$(QEMU) $(QEMU_DISK) $(QEMU_DEBUG)

.PHONY: qemub
qemub: $(BUILD)/kernel.iso
	$(QEMU) $(QEMU_CDROM) \
	# $(QEMU_DEBUG)
