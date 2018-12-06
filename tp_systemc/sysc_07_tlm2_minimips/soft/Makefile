SOURCES=$(wildcard *.s)
TARGETS=$(SOURCES:%.s=%.bin) $(SOURCES:%.s=%.bin.txt)

# .PHONY targets don't generate files
.PHONY:	all clean

# Default target
all: $(TARGETS)

%.bin: %.s
	mipsel-unknown-elf-as -o $@ $<

%.bin.txt: %.bin
	mipsel-unknown-elf-objdump -D $< > $@

clean:
	-rm -f *.bin *.bin.txt
