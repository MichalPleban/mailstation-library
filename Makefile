CC = sdcc
INCLUDES = -Iinclude
CFLAGS = $(INCLUDES) -mz80
LDFLAGS = --no-std-crt0
AS = sdasz80
ASFLAGS = -lso
ASFLAGS_CRT0 := $(ASFLAGS) -g

all: crt0 lib app

.PHONY: crt0 app clean

################################################################################
# CRT0 files for building applications
################################################################################

crt0: bin/crt0-app.rel

bin/crt0-%.rel: src/crt0/crt0-%.s
	$(AS) $(ASFLAGS_CRT0) $@ $<
	rm bin/crt0-$*.lst bin/crt0-$*.sym

################################################################################
# The library
################################################################################

LIB_OBJECTS = src/lib/init.rel src/lib/screen.rel src/lib/irq.rel src/lib/irq_handler.rel src/lib/port.rel src/lib/font.rel src/lib/kbd.rel

lib: bin/libmailstation.lib

bin/libmailstation.lib: $(LIB_OBJECTS)
	sdar -rc $@ $^

src/lib/init.rel: src/lib/init.c include/mailstation.h
	$(CC) $(CFLAGS) -c $< -o $@

src/lib/screen.rel: src/lib/screen.c include/mailstation.h
	$(CC) $(CFLAGS) -c $< -o $@

src/lib/irq.rel: src/lib/irq.c include/mailstation.h
	$(CC) $(CFLAGS) -c $< -o $@

src/lib/irq_handler.rel: src/lib/irq_handler.s include/mailstation.h
	$(AS) $(ASFLAGS) -o $@ $<

src/lib/port.rel: src/lib/port.c include/mailstation.h
	$(CC) $(CFLAGS) -c $< -o $@

src/lib/kbd.rel: src/lib/kbd.c include/mailstation.h
	$(CC) $(CFLAGS) -c $< -o $@

src/lib/font.rel: src/lib/font.c include/mailstation.h
	$(CC) $(CFLAGS) -c $< -o $@

################################################################################
# Test application
################################################################################

app: bin/app.bin bin/dataflash.bin bin/app-ldr.bin

bin/app-ldr.bin: bin/app.bin
	cat bin/trampoline.bin bin/app.bin > bin/app-ldr.bin

bin/app.bin: bin/app.ihx
	objcopy -Iihex -Obinary $< $@

bin/app.ihx: bin/crt0-app.rel bin/libmailstation.lib src/app/app.rel src/app/icon.rel src/app/ports.rel
	$(CC) $(CFLAGS) $(LDFLAGS) --code-loc 0x4030 --data-loc 0xD700 $^ -o $@

src/app/app.rel: src/app/app.c include/mailstation.h
	$(CC) $(CFLAGS) -c $< -o $@

src/app/icon.rel: src/app/icon.c include/mailstation.h
	$(CC) $(CFLAGS) -c $< -o $@

src/app/ports.rel: src/app/ports.c include/mailstation.h
	$(CC) $(CFLAGS) -c $< -o $@

bin/dataflash.bin: bin/app.bin
	tools/install_app.py --file bin/app.bin --dataflash bin/dataflash.bin

