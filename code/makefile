#http://theory.uwinnipeg.ca/localfiles/infofiles/make/make_27.html

PROCESSOR_FLAG=-mmcs51
CC=sdcc
CFLAGS=--code-size 8096 --iram-size 256 --xram-size 2048 --xram-loc 0x2000 --no-xinit-opt --main-return --debug
VPATH = SRC;../;./output/
DEST = -o .\output\\

PROJ = VaporB72.c	
SRCS = $(PROJ) \
	CloseBolt.c \
	CommonFiring.c \
	Extras.c \
	Eyes.c \
	ISRs.c \
	Led.c \
	Memory.c \
	OpenBolt.c \
	Ramp.c \
	Trigger.c

# Define all object files.
OBJS := $(SRCS:.c=.rel)
#OBJS := $(RELS:%.rel,$(DEST)%.rel)

#HDRS := $(SRCS:.c=.h) Globals.h Macros.h VaporIo.h

DEFS := $(SRCS:.c=.d)
		

all: rMod

rMod: $(OBJS)
	echo Building final output
	$(CC) $(PROCESSOR_FLAG) $(CFLAGS) $(OBJS)

.PHONEY : clean

clean:
	$(RM) *.asm *.cod *.lst *.p *.d *.lnk *~ *.rel *.sym *.rst *.cdb *.adb
	
$(OBJS): $(SRCS)
	echo Building rel for $@
	$(CC) $(PROCESSOR_FLAG) $(CFLAGS) -c $(@:.rel=.c)

$(DEFS): $(*:.d=.c)
	echo Building .d for $@
	$(SHELL) -ec \
	'$(CC) -MM $(@:.d=.c) \
        | sed '\''s/\($*\)\.rel[ :]*/\1.rel $@ : /g'\'' > $@; \
        [ -s $@ ] || rm -f $@'
		
include $(DEFS)
