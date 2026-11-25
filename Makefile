ifeq ($(strip $(DEVKITARM)),)
$(error "Set DEVKITARM, e.g. export DEVKITARM=/opt/devkitpro/devkitARM")
endif
DEVKITPRO ?= /opt/devkitpro
LIBNDS    := $(DEVKITPRO)/libnds
CALICO    := $(DEVKITPRO)/calico

TARGET  := crypt-delver
SRC     := $(wildcard source/*.c) $(wildcard *.c)
OBJ     := $(SRC:.c=.o)

PREFIX  := $(DEVKITARM)/bin/arm-none-eabi-
CC      := $(PREFIX)gcc
ARCH    := -march=armv5te -mtune=arm946e-s -mthumb -mthumb-interwork
CFLAGS  := -O2 -Wall -std=gnu11 $(ARCH) -D__NDS__ -DARM9 \
	   -I$(LIBNDS)/include -I$(CALICO)/include \
	   -ffunction-sections -fdata-sections
LDFLAGS := -specs=$(CALICO)/share/ds9.specs $(ARCH) \
	   -L$(LIBNDS)/lib -L$(CALICO)/lib
LIBS    := -lnds9 -lcalico_ds9
ARM7    := $(CALICO)/bin/ds7_maine.elf

all: $(TARGET).nds

$(TARGET).nds: $(TARGET).elf
	$(DEVKITPRO)/tools/bin/ndstool -c $@ -7 $(ARM7) -9 $< $(if $(wildcard icon.bmp),-b icon.bmp "Crypt Delver;Dungeon roguelike;Homebrew")

$(TARGET).elf: $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET).elf $(TARGET).nds

.PHONY: all clean
