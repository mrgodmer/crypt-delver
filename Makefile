ifeq ($(strip $(DEVKITARM)),)
$(error "Set DEVKITARM, e.g. export DEVKITARM=/opt/devkitpro/devkitARM")
endif
DEVKITPRO ?= /opt/devkitpro
LIBNDS    := $(DEVKITPRO)/libnds

TARGET  := crypt-delver
SRC     := $(wildcard source/*.c) $(wildcard *.c)
OBJ     := $(SRC:.c=.o)

PREFIX  := $(DEVKITARM)/bin/arm-none-eabi-
CC      := $(PREFIX)gcc
ARCH    := -march=armv5te -mtune=arm946e-s -mthumb -mthumb-interwork
CFLAGS  := -O2 -Wall -std=gnu11 $(ARCH) -DARM9 -I$(LIBNDS)/include \
           -ffunction-sections -fdata-sections
LDFLAGS := -specs=ds_arm9.specs $(ARCH) -L$(LIBNDS)/lib
LIBS    := -lnds9

all: $(TARGET).nds

$(TARGET).nds: $(TARGET).elf
	$(DEVKITPRO)/tools/bin/ndstool -c $@ -9 $<

$(TARGET).elf: $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET).elf $(TARGET).nds

.PHONY: all clean
