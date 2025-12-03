# PATHS
MSPGCC_ROOT_PATH = $(HOME)/msp430/msp430-gcc-9.3.1.11_linux64
GCC_DIR = $(MSPGCC_ROOT_PATH)/bin
MSPGCC_MSP430_PATH = $(MSPGCC_ROOT_PATH)/include
MSPGCC_LIB_PATH = $(MSPGCC_ROOT_PATH)/include

BUILD_PATH = build
OBJ_PATH = $(BUILD_PATH)/obj
BIN_PATH = $(BUILD_PATH)/bin

INCLUDE_PATHS = $(MSPGCC_LIB_PATH)
LIB_PATHS= $(MSPGCC_LIB_PATH)

CC = $(GCC_DIR)/msp430-elf-gcc
GDB = $(GCC_DIR)/msp430-elf-gdb

TARGET = $(BIN_PATH)/blink.elf

SOURCES = main.c \
		  led.c


OBJECTS = $(patsubst %.c, $(OBJ_PATH)/%.o, $(SOURCES))

MCU = msp430g2553
WFLAGS = -Wall -Werror -Wshadow
CFLAGS = -mmcu=$(MCU) $(WFLAGS) $(addprefix -I, $(INCLUDE_PATHS)) -Og -g
LDFLAGS = -mmcu=$(MCU) $(addprefix -L, $(LIB_PATHS))

$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_PATH)
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJ_PATH)/%.o: %.c
	@mkdir -p $(OBJ_PATH)
	$(CC) $(CFLAGS) -c -o $@ $<


.PHONY: all clean

all: $(TARGET)

clean:
	rm -rf $(BUILD_PATH)

flash: $(TARGET)
	mspdebug rf2500 "prog $(TARGET)"