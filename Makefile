PROJECT  = ads8321
MCU      = atmega328p
OPT      = 2
TARGET   = $(PROJECT)
F_CPU    = 16000000

# Toolchain definitions
TRGT   = avr-
CC     = $(TRGT)gcc
CP     = $(TRGT)objcopy
AS     = $(TRGT)gcc -x assembler-with-cpp
HEX    = $(CP) -O ihex
SIZE   = $(TRGT)size
BIN    = $(CP) -O binary

### build sub dirs ###
BUILDDIR = build/
OBJDIR   = $(BUILDDIR)obj/
DEPDIR   = $(BUILDDIR)dep/

# C sources 
vpath %.c src/
CSOURCES  = main.c
CSOURCES += uart.c

INCLUDES  = ./src/


# Pre-processor defines
DEFINES = -DF_CPU=$(F_CPU)

DEFINES += -DSOFTWARE_VERSION=1

# Programmer definitions
PROGCMD = avrdude
PROGFLAGS = -c dragon_isp -p m328p  
#PROGFLAGS = -c avrisp2  -p m328p  
PROGFILE = -e -U flash:w:$(BUILDDIR)$(TARGET).hex


# OS check
UNAME = $(shell uname)

# C compiler flags
CFLAGS += -mmcu=$(MCU) -O$(OPT) -Wa,-ahlms=$(@:.o=.lst)

# debug flags
CFLAGS += -g -g3 -ggdb

CFLAGS += -std=gnu99

CFLAGS += -ffunction-sections -fdata-sections -fshort-enums

include warnings.mk

# warnings enabled
CFLAGS += -Wall -Wextra 
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wformat=2 -Wswitch-default -Wswitch-enum -Wcast-align 
CFLAGS += -Wbad-function-cast -Wstrict-overflow=5 -Wstrict-prototypes
CFLAGS += -Winline -Wundef -Wnested-externs -Wcast-qual 
CFLAGS += -Wshadow -Wunreachable-code -Wuninitialized
CFLAGS += -Wlogical-op -Wfloat-equal -Wstrict-aliasing=2 -Wredundant-decls
CFLAGS += -Wold-style-definition -Wmissing-include-dirs -Wunused 
CFLAGS += -Wstack-usage=256 -Wvla
CFLAGS += -Wmissing-declarations

# warnings disabled
CFLAGS += -Wno-div-by-zero

CFLAGS += $(DEFINES)

# Linker flags
LDFLAGS  = -mmcu=$(MCU) 
LDFLAGS += -Wl,-Map=$(BUILDDIR)$(TARGET).map,--cref
LDFLAGS += -Wl,--gc-sections -Wl,--start-group -Wl,--end-group

# Dependency generation flags
# -MT $(@:.c=.o)
DEPFLAGS = -mmcu=$(MCU) -M -MF $(DEPDIR)$*.dep

INCS = $(addprefix -I,$(INCLUDES))

OBJECTS   = $(CSOURCES:.c=.o) $(ASOURCES:.S=.o)
BUILDOBJS = $(addprefix $(OBJDIR),$(OBJECTS))

# OS fix! Since cmd includes a mkdir command and this may or may not
#         interpret '/' as a directory separator
ifeq ($(UNAME), windows32)
 MKDIR    = test -d "$(dir $(DEPDIR)$*)" || mkdir "$(dir $(DEPDIR)$*)" 
 MKDIR   += && test -d "$(dir $(OBJDIR)$*)" || mkdir "$(dir $(OBJDIR)$*)"
else
 MKDIR    = mkdir -p $(dir $(DEPDIR)$*) $(dir $(OBJDIR)$*)
endif

# Dependency generation command
MKDEPEND = $(CC) $(DEPFLAGS) $(INCS) $(DEFINES) $<
MKDIRDEP = $(MKDIR); $(MKDEPEND)


.PHONY: all elf hex allclean install

all: elf bin hex $(BUILDOBJS) 
	@echo -n
	@echo "==========================================================="
	@echo "  -  Project build complete                                "
	@echo "==========================================================="
	@echo -n
	@$(CC) --version
	@echo Application:
	@$(SIZE) $(BUILDOBJS)
	@echo Output:
	@$(SIZE) $(BUILDDIR)$(TARGET).elf

allclean:
	make clean
	make all
	

%.hex: %.bin
	@$(CP) -R .eeprom -R .fuse -R .lock -R .signature \
		-I binary -O ihex $(BUILDDIR)$< $(BUILDDIR)$@
	
%.bin: %.elf
	@$(CP) -O binary $(BUILDDIR)$< $(BUILDDIR)$@
	
%.elf: $(BUILDOBJS)
	@$(CC) $(CFLAGS) $(LDFLAGS) $(BUILDOBJS) -o $(BUILDDIR)$@ $(LDMATHFLAG)

hex: $(TARGET).hex elf
	@echo "  -  Intel Hex file created "

bin: $(TARGET).bin elf
	@echo "  -  Binary file created "
	
elf: $(TARGET).elf
	@echo "  -  ELF binary file created "
	
flash: hex
	$(PROGCMD) $(PROGFLAGS) $(PROGFILE) $(PROGCMDLINE)
	@echo -n
	@echo "Firmware flash complete"
	@echo -n

install: hex
	avrdude -p m328p -c arduino -b 57600 -P /dev/ttyUSB0 \
	-U flash:w:$(BUILDDIR)$(TARGET).hex

fuse:
	$(PROGCMD) $(PROGFLAGS) -U lfuse:w:0x77:m
	@echo ""
	
	
$(OBJDIR)%.o: $(SOURCEDIR)%.c
	@$(MKDIR)
	@$(MKDEPEND)
	@$(CC) $(CFLAGS) $(INCS) -c -o $@ $<
	
clean:
	-rm -rf $(BUILDDIR)*
	-rm -f $(VERSION_H_PATH)

-include $(addprefix $(DEPDIR),$(OBJECTS:.o=.dep))

