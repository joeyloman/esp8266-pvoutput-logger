all : image.elf
FW_FILE_1:=0x00000.bin
FW_FILE_2:=0x40000.bin

TARGET_OUT:=image.elf
OBJS:=driver/uart.o \
	user/led.o \
	user/interrupt.o \
	user/date_time.o \
	user/queue.o \
	user/scheduler.o \
	user/pvoutput_client.o \
	user/thingspeak_client.o \
	user/wifi.o \
	user/user_main.o

SRCS:=driver/uart.c \
	user/led.c \
	user/interrupt.c \
	user/date_time.c \
	user/queue.c \
	user/scheduler.c \
	user/pvoutput_client.c \
	user/thingspeak_client.c \
	user/wifi.c \
	user/user_main.c 

ESP_DEVEL_FOLDER:=$(HOME)/esp-devel
GCC_FOLDER:=$(ESP_DEVEL_FOLDER)/esp-open-sdk/xtensa-lx106-elf
ESPTOOL_PY:=$(ESP_DEVEL_FOLDER)/esptool/esptool.py
FW_TOOL:=$(ESP_DEVEL_FOLDER)/other/esptool/esptool
SDK:=$(ESP_DEVEL_FOLDER)/esp_iot_sdk/esp_iot_sdk-latest
PORT:=/dev/ttyUSB0

XTLIB:=$(SDK)/lib
XTGCCLIB:=$(GCC_FOLDER)/lib/gcc/xtensa-lx106-elf/4.8.5/libgcc.a
FOLDERPREFIX:=$(GCC_FOLDER)/bin
PREFIX:=$(FOLDERPREFIX)/xtensa-lx106-elf-
CC:=$(PREFIX)gcc

CFLAGS:=-mlongcalls -I$(SDK)/include -Iinclude -Os

LDFLAGS_CORE:=\
	-nostdlib \
	-Wl,--relax -Wl,--gc-sections \
	-L$(XTLIB) \
	-L$(XTGCCLIB) \
	$(GCC_FOLDER)/xtensa-lx106-elf/lib/libc.a \
	$(SDK)/lib/liblwip.a \
	$(SDK)/lib/libwpa.a \
	$(SDK)/lib/libcrypto.a \
	$(SDK)/lib/libphy.a \
	$(SDK)/lib/libmain.a \
	$(SDK)/lib/libnet80211.a \
	$(SDK)/lib/libpp.a \
	$(XTGCCLIB) \
	-T $(SDK)/ld/eagle.app.v6.ld

LINKFLAGS:= \
	$(LDFLAGS_CORE) \
	-B$(XTLIB)

$(TARGET_OUT) : $(SRCS)
	$(PREFIX)gcc $(CFLAGS) $^  -flto $(LINKFLAGS) -o $@

$(FW_FILE_1): $(TARGET_OUT)
	@echo "FW $@"
	$(FW_TOOL) -eo $(TARGET_OUT) -bo $@ -bs .text -bs .data -bs .rodata -bc -ec

$(FW_FILE_2): $(TARGET_OUT)
	@echo "FW $@"
	$(FW_TOOL) -eo $(TARGET_OUT) -es .irom0.text $@ -ec

flash: $(FW_FILE_1) $(FW_FILE_2)
	($(ESPTOOL_PY) --port $(PORT) write_flash 0x00000 0x00000.bin 0x40000 0x40000.bin)||(true)

clean:
	rm -rf user/*.o driver/*.o $(TARGET_OUT) $(FW_FILE_1) $(FW_FILE_2)
