
export CC_TARGET=arm-eabi
export FW_BASEDIR=$(shell pwd)
export CC=$(CC_TARGET)-gcc
export AS=$(CC_TARGET)-as
export LD=$(CC_TARGET)-ld
export AR=$(CC_TARGET)-ar
export OBJDUMP=$(CC_TARGET)-objdump
export ELF2ROM=$(FW_BASEDIR)/elf2rom


export LIBS_PATH=$(FW_BASEDIR)/lib
export INCLUDE_PATH1=$(FW_BASEDIR)
export INCLUDE_PATH2=$(FW_BASEDIR)/contrib/mpdecimal-2.4.0/libmpdec
export DIR_LIST= contrib/mpdecimal-2.4.0/libmpdec
export SRCS= $(wildcard *.c) \
    contrib/mpdecimal-2.4.0/libmpdec/basearith.c \
    contrib/mpdecimal-2.4.0/libmpdec/constants.c \
    contrib/mpdecimal-2.4.0/libmpdec/context.c \
    contrib/mpdecimal-2.4.0/libmpdec/convolute.c \
    contrib/mpdecimal-2.4.0/libmpdec/crt.c \
    contrib/mpdecimal-2.4.0/libmpdec/difradix2.c \
    contrib/mpdecimal-2.4.0/libmpdec/fnt.c \
    contrib/mpdecimal-2.4.0/libmpdec/fourstep.c \
    contrib/mpdecimal-2.4.0/libmpdec/io.c \
    contrib/mpdecimal-2.4.0/libmpdec/memory.c \
    contrib/mpdecimal-2.4.0/libmpdec/mpdecimal.c \
    contrib/mpdecimal-2.4.0/libmpdec/mpsignal.c \
    contrib/mpdecimal-2.4.0/libmpdec/numbertheory.c \
    contrib/mpdecimal-2.4.0/libmpdec/sixstep.c \
    contrib/mpdecimal-2.4.0/libmpdec/transpose.c

export OBJS= $(SRCS:.c=.o)

# Common flags
export ARM_ELF_CFLAGS= -mtune=arm920t -mcpu=arm920t \
	-mlittle-endian -fomit-frame-pointer -msoft-float -Wall \
	-Os -pipe -D NDEBUG -D SLIM_MPD -mthumb-interwork -I$(INCLUDE_PATH1) -I$(INCLUDE_PATH2)

export ARM_ELF_ASFLAGS= -EL -k -mcpu=arm920t -mno-fpu -mthumb-interwork

export ARM_ELF_LDFLAGS= -nodefaultlibs -nostdlib


all: libnewrpl.a

libnewrpl.a : $(OBJS)
	@echo Sources= $(SRCS)
	$(AR) -r libnewrpl.a $(OBJS)

clean:
	-@rm -f $(OBJS) *.a

	
install: all
	echo Nothing to install


%.o : %.c
	@$(CC) $(ARM_ELF_CFLAGS) -marm -c $< -o $@ 

