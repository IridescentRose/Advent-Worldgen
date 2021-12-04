TARGET = Worldgen
OBJS = src/benchmark.o \
src/setup.o \
src/gfx.o \
src/worldgen.o \
src/main.o

INCDIR = $(PSPPATH)/include
INCDIR += 
CFLAGS = -Wall -Ofast -G0
CXXFLAGS = -std=gnu++17
ASFLAGS = $(CFLAGS)
LIBS= -lstdc++


# PSP Stuff
BUILD_PRX = 1
PSP_FW_VERSION = 500
PSP_LARGE_MEMORY = 1
 
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Worldgen

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak