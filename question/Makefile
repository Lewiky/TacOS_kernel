# part 1: variables

 PROJECT_PATH     = $(shell find . -mindepth 1 -maxdepth 1 -type d)
 PROJECT_SOURCES  = $(shell find ${PROJECT_PATH} -name *.c -o -name *.s)
 PROJECT_HEADERS  = $(shell find ${PROJECT_PATH}              -name *.h)
 PROJECT_OBJECTS  = $(addsuffix .o, $(basename ${PROJECT_SOURCES}))
 PROJECT_TARGETS  = image.elf image.bin

 QEMU_PATH        = /usr/local/bin
 QEMU_GDB         =        127.0.0.1:1234
 QEMU_UART        = stdio
 QEMU_UART       += telnet:127.0.0.1:1235,server
 QEMU_UART       += telnet:127.0.0.1:1236,server

 GCC_PATH      = ../../gcc-arm-none-eabi-5_2-2015q4
 GCC_PREFIX    = arm-none-eabi

 DISK_FILE        = disk.bin
 DISK_HOST        = 127.0.0.1
 DISK_PORT        = 1236
 DISK_BLOCK_NUM   = 65536
 DISK_BLOCK_LEN   =    12

# part 2: build commands

%.o   : %.s
	@${GCC_PATH}/bin/${GCC_PREFIX}-as  $(addprefix -I , ${PROJECT_PATH} ${GCC_PATH}/arm-eabi/include) -mcpu=cortex-a8                                       -g       -o ${@} ${<}
%.o   : %.c
	@${GCC_PATH}/bin/${GCC_PREFIX}-gcc $(addprefix -I , ${PROJECT_PATH} ${GCC_PATH}/arm-eabi/include) -mcpu=cortex-a8 -mabi=aapcs -ffreestanding -std=gnu99 -g -c -O -o ${@} ${<}

%.elf : ${PROJECT_OBJECTS}
	@${GCC_PATH}/bin/${GCC_PREFIX}-ld  $(addprefix -L , ${GCC_PATH}/lib/gcc/arm-none-eabi/5.2.1) $(addprefix -L , ${GCC_PATH}/arm-none-eabi/lib) -T ${*}.ld -o ${@} ${^} -lc -lgcc
%.bin : %.elf
	@${GCC_PATH}/bin/${GCC_PREFIX}-objcopy -O binary ${<} ${@}

# part 3: targets

.PRECIOUS   : ${PROJECT_OBJECTS} ${PROJECT_TARGETS}

build       : ${PROJECT_TARGETS}

launch-qemu : ${PROJECT_TARGETS}
	@${QEMU_PATH}/qemu-system-arm -M realview-pb-a8 -m 128M -display none -gdb tcp:${QEMU_GDB} $(addprefix -serial , ${QEMU_UART}) -S -kernel $(filter %.bin, ${PROJECT_TARGETS})

launch-gdb  : ${PROJECT_TARGETS}
	@${GCC_PATH}/bin/${GCC_PREFIX}-gdb -ex "file $(filter %.elf, ${PROJECT_TARGETS})" -ex "target remote ${QEMU_GDB}"

launch-console:
	@nc 127.0.0.1 1235

kill-qemu   :
	@-killall -u ${USER} qemu-system-arm > /dev/null 2>&1 || true

kill-gdb    :
	@-killall -u ${USER} ${GCC_PREFIX}-gdb > /dev/null 2>&1 || true

clean       : kill-qemu kill-gdb
	@rm -f core ${PROJECT_OBJECTS} ${PROJECT_TARGETS}

# customer launcher using applescript for iTerm 2
launch:
	@osascript launcher.scpt

relaunch:
	@osascript relauncher.scpt

create-disk :
	@dd of=${DISK_FILE} if=/dev/zero count=${DISK_BLOCK_NUM} bs=${DISK_BLOCK_LEN}

inspect-disk :
	@hexdump  -C ${DISK_FILE}

launch-disk :
	@python2 device/disk.py --host=${DISK_HOST} --port=${DISK_PORT} --file=${DISK_FILE} --block-num=${DISK_BLOCK_NUM} --block-len=${DISK_BLOCK_LEN} --debug
