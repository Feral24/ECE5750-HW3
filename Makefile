#!/usr/bin/gmake

CC = gcc
AS = $(CC)

M4 	  := m4 
M4FLAGS   := -Ulen -Uindex
MACS	  := c.m4.ece5750

FORMAT_DIR = .
DEFINES =
INCLUDES =
CFLAGS = -m32 -static
ASFLAGS = -c -m32
LIBS = -lm

.SILENT:

TARGET = my_lock
OFILESFROMC = my_lock.o
OFILESFROMASM = lock.o
OFILES = $(OFILESFROMC) $(OFILESFROMASM)

.SECONDARY: $(OFILESFROMC:%.o=%.c)

All: prolog $(TARGET) 

$(TARGET): $(OFILESFROMC) $(OFILESFROMASM)
	@echo "Linking $@"
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -o $@ $(OFILES) $(LIBS)

$(OFILESFROMC:%.o=%.c): $(MACS)

.C.c: 
	@echo "Macro expanding $<"
	$(M4) $(M4FLAGS) $(MACS) $< > $@

.c.o:
	@echo "Compiling $<"
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $<

.s.o:
	@echo "Assembling $<"
	$(AS) $(ASFLAGS) $(DEFINES) -o $@ $<

nice:
	@echo "Nicing..."
	\rm -f *~ \#* .\#* *.bak

clean: nice
	@echo "Cleaning..."
	\rm -f $(OFILES) *.c

clobber: clean
	@echo "Clobbering..."
	\rm -f $(TARGET)

prolog:
	@echo ""
	@echo "   ANL_Support"
	@echo ""
	@echo "   Building $(TARGET)"
	@echo ""
	@echo "   M4 Flags  : `perl $(FORMAT_DIR)/format.pl $(M4FLAGS)`"
	@echo "   C Flags   : `perl $(FORMAT_DIR)/format.pl $(CFLAGS)`"
	@echo "   AS Flags  : `perl $(FORMAT_DIR)/format.pl $(ASFLAGS)`"
	@echo "   Defines   : `perl $(FORMAT_DIR)/format.pl $(DEFINES)`"
	@echo "   Includes  : `perl $(FORMAT_DIR)/format.pl $(INCLUDES)`"
	@echo "   Libraries : `perl $(FORMAT_DIR)/format.pl $(LIBS)`"
	@echo ""
