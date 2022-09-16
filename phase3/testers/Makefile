#$Id: Makefile,v 1.2 2004/05/01 14:53:48 morsiani Exp morsiani $
# Makefile for mipsel-linux
#
ifneq ($(wildcard /usr/bin/umps3),)
	UMPS3_DIR_PREFIX = /usr
else
	UMPS3_DIR_PREFIX = /usr/local
endif

LIBDIR = $(UMPS3_DIR_PREFIX)/lib/umps3
INCDIR = $(UMPS3_DIR_PREFIX)/include/umps3/umps
SUPDIR = $(UMPS3_DIR_PREFIX)/share/umps3

TDEFS = h/print.h h/tconst.h $(INCDIR)/libumps.e Makefile

CFLAGS = -ffreestanding -ansi -c -mips1 -mabi=32 -mfp32 -mno-gpopt -G 0 -I/usr/include/umps3 -std=gnu99 -fno-pic -mno-abicalls
# -Wall

LDAOUTFLAGS = -G 0 -nostdlib -T $(SUPDIR)/umpsaout.ldscript
LDCOREFLAGS =  -G 0 -nostdlib -T $(SUPDIR)/umpscore.ldscript

CC = mipsel-linux-gnu-gcc
LD = mipsel-linux-gnu-ld
AS = mipsel-linux-gnu-as -KPIC

EF = umps3-elf2umps
UDEV = umps3-mkdev

#main target
all: printerTest.umps strConcat.umps \
	fibEight.umps fibEleven.umps \
	terminalTest2.umps terminalTest3.umps terminalTest4.umps \
	terminalTest5.umps  
	
%.o: %.c $(TDEFS)
	$(CC) $(CFLAGS) $<
	
%.t: %.o print.o  $(LIBDIR)/crti.o
	$(LD) $(LDAOUTFLAGS) $(LIBDIR)/crti.o $< print.o $(LIBDIR)/libumps.o -o $@
	
%.t.aout.umps: %.t
	$(EF) -a $<

%.umps: %.t.aout.umps
	$(UDEV) -f $@ $<


clean:
	rm -f *.o *.t *.umps

