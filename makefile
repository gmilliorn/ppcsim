# List of macros:

DEBUG  	=  
CC 	= gcc
LD 	= gcc

CC_INC = 
CFLAGS = $(CC_INC) -g -c -O -Wno-format-security
AFLAGS = 


# Simulator
#
PPCSIM_GM	= main breakpt mem symbols file shell cpu system nb mss uart
PPCSIM_MOT	= ppcsim regs spr asm dasm 
PPCSIM_MOD	= $(PPCSIM_GM) $(PPCSIM_MOT)
PPCSIM_SRC	= $(PPCSIM_MOD:%=%.c)
PPCSIM_OBJ	= $(PPCSIM_MOD:%=%.o)



ppcsim:	$(PPCSIM_OBJ)
	$(LD) $(LDFLAGS) $(PPCSIM_OBJ) $(LDLIBS) -o ppcsim -lm

clean:
	rm *.o

main.o: main.c ppcsim.h clist.h
	$(CC) $(CFLAGS) $(@F:%.o=%.c) -o $@

breakpt.o: breakpt.c ppcsim.h
	$(CC) $(CFLAGS) $(@F:%.o=%.c) -o $@

mem.o: mem.c ppcsim.h
	$(CC) $(CFLAGS) $(@F:%.o=%.c) -o $@

symbols.o: symbols.c ppcsim.h
	$(CC) $(CFLAGS) $(@F:%.o=%.c) -o $@

file.o: file.c ppcsim.h
	$(CC) $(CFLAGS) $(@F:%.o=%.c) -o $@

shell.o: shell.c ppcsim.h
	$(CC) $(CFLAGS) $(@F:%.o=%.c) -o $@

cpu.o: cpu.c ppcsim.h
	$(CC) $(CFLAGS) $(@F:%.o=%.c) -o $@

system.o: system.c ppcsim.h
	$(CC) $(CFLAGS) $(@F:%.o=%.c) -o $@

nb.o: nb.c ppcsim.h nb.h
	$(CC) $(CFLAGS) $(@F:%.o=%.c) -o $@

mss.o: mss.c ppcsim.h
	$(CC) $(CFLAGS) $(@F:%.o=%.c) -o $@

uart.o: uart.c ppcsim.h
	$(CC) $(CFLAGS) $(@F:%.o=%.c) -o $@

ppcsim.o: ppcsim.c ppcsim.h
	$(CC) $(CFLAGS) $(@F:%.o=%.c) -o $@

regs.o: regs.c ppcsim.h
	$(CC) $(CFLAGS) $(@F:%.o=%.c) -o $@

spr.o: spr.c ppcsim.h regs_spr.h
	$(CC) $(CFLAGS) $(@F:%.o=%.c) -o $@

asm.o: asm.c ppcsim.h asm_dsm.h
	$(CC) $(CFLAGS) $(@F:%.o=%.c) -o $@

dasm.o: dasm.c ppcsim.h asm_dsm.h
	$(CC) $(CFLAGS) $(@F:%.o=%.c) -o $@
