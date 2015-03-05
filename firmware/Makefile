
export CC_TARGET=arm-eabi
export FW_BASEDIR=/usr/home/user/Programs/firmware
export CC=$(CC_TARGET)-gcc
export AS=$(CC_TARGET)-as
export LD=$(CC_TARGET)-ld
export AR=$(CC_TARGET)-ar
export OBJDUMP=$(CC_TARGET)-objdump
export ELF2ROM=$(FW_BASEDIR)/elf2rom


export LIBS_PATH=$(FW_BASEDIR)/lib
export INCLUDE_PATH=$(FW_BASEDIR)/include
export NEWRPL_PATH=$(FW_BASEDIR)/../newrpl-base
export NEWRPL_INCLUDE=$(FW_BASEDIR)/../newrpl-base
export MPDEC_INCLUDE=$(FW_BASEDIR)/../newrpl-base/contrib/mpdecimal-2.4.0/libmpdec
export FILE_LIST= fw_base.o
export DIR_LIST= ggl ../newrpl-base
export LIBS_LIST= -lggl -lnewrpl
export PREAMBLE=fw_preamble.o

# Common flags
export ARM_ELF_CFLAGS= -mtune=arm920t -mcpu=arm920t \
	-mlittle-endian -fomit-frame-pointer -msoft-float -Wall \
	-Os -pipe -mthumb-interwork -I$(INCLUDE_PATH) -I$(NEWRPL_INCLUDE) -I$(MPDEC_INCLUDE)

export ARM_ELF_ASFLAGS= -EL -k -mcpu=arm920t -mno-fpu -mthumb-interwork

export ARM_ELF_LDFLAGS= -nodefaultlibs -nostdlib


clean:
	-@rm -f *.o *.elf *.bin
	@for I in ${DIR_LIST}; do cd $$I && $(MAKE) $@ && cd ..; done;

	
install: allfiles
	echo Nothing to install

all: allfiles


rom.bin : allfiles
	@for I in ${DIR_LIST}; do cd $$I && $(MAKE) && $(MAKE) install && cd ..; done;
	$(LD) -Tld.script ${ARM_ELF_LDFLAGS}  $(FILE_LIST) -L$(LIBS_PATH) -L$(NEWRPL_PATH) $(LIBS_LIST) -lgcc -o rom.elf
	$(OBJDUMP) -D rom.elf > rommap.txt
	$(ELF2ROM) rom.elf rom.bin

allfiles: $(FILE_LIST) $(PREAMBLE)

fw_base.o : fw_base.c
	@$(CC) $(ARM_ELF_CFLAGS) -marm -c $< -o $@ 

fw_payload.o : fw_payload.c
	@$(CC) $(ARM_ELF_CFLAGS) -mthumb -c $< -o $@
