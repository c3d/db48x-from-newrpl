#******************************************************************************
# Makefile                                                        DB48X project
#******************************************************************************
#
#  File Description:
#
##    Makefile to build the various possible targets of the DB48X project
##      Avaiable targets include:
##       % make sim:             Build all simulators
##       % make fw:              Build all firmwares
##       % make all:             Build simulators and firmwares
##       % make <name>-sim:      Build one simulator
##       % make <name>-fw:       Build one firmware
##       % make debug-<target>   Build with debugging enabled
##       % make release-<target> Build release, debugging disabled
##       % make help:            Show this message
#
#
#******************************************************************************
#  (C) 2022 Christophe de Dinechin <christophe@dinechin.org>
#  (C) 2022 Claudio Lapilli and the newRPL team
#  This software is licensed under the terms described in LICENSE.txt
#******************************************************************************

SIMULATORS=hp50 prime dm42
FIRMWARES=hp39 hp40 hp48 hp50 prime prime-multiload
CONFIG=debug
OS=$(shell uname -s)
TAG=$(OS)-$(CONFIG)

sim simulators: $(SIMULATORS:%=%-simulator)
fw firmwares:  $(FIRMWARES:%=%-firmware)
all: simulators firmwares

help:
	@sed -E -e '/^##/s/^##\s?//g;t' -e d $(MAKEFILE_LIST)
	@echo "    Available simulators: $(SIMULATORS)"
	@echo "    Available firmwares:  $(FIRMWARES)"

compiler: compiler-$(TAG).mak helpfile recorder
	$(MAKE) -f $< install

elf2rom: tools-bin/elf2rom
tools-bin/elf2rom: tools/elf2rom/elf2rom.mak | compiler
	cd tools/elf2rom && $(MAKE) -f elf2rom.mak install
tools/elf2rom/elf2rom.mak: tools/elf2rom/elf2rom.pro
	cd tools/elf2rom && qmake $(<F) -o $(@F)

bmp2font: tools-bin/bmp2font
tools-bin/bmp2font: tools/fonts/bmp2font/bmp2font.mak tools/fonts/bmp2font/main.c
	cd tools/fonts/bmp2font && $(MAKE) -f bmp2font.mak install
tools/fonts/bmp2font/bmp2font.mak: tools/fonts/bmp2font/bmp2font.pro
	cd tools/fonts/bmp2font && qmake $(<F) -o $(@F)

ttf2font: tools-bin/ttf2font
tools-bin/ttf2font: tools/fonts/ttf2font/ttf2font.mak tools/fonts/ttf2font/ttf2font.c
	cd tools/fonts/ttf2font && $(MAKE) -f ttf2font.mak install
tools/fonts/ttf2font/ttf2font.mak: tools/fonts/ttf2font/ttf2font.pro
	cd tools/fonts/ttf2font && qmake $(<F) -o $(@F)

doc docs: tools/extractcmd/extractcmd
	$< newrpl -m doc/commands/

helpfile: firmware/helpfile.inc
firmware/helpfile.inc: $(wildcard doc/calc-help/*.md doc/commands/*.md)
	cat $^ |				\
	sed > $@				\
	    -e 's/"/\\"/g'			\
	    -e 's/^\*\(.*\)$$/"·\1\\n"/g;t'	\
	    -e 's/^#\(.*\)$$/"#\1\\n"/g;t'	\
	    -e 's/^[ ]*$$/"\\n"/g;t'		\
	    -e 's/^\(.*\)$$/"\1 "/g'

tools/extractcmd/extractcmd: tools/extractcmd/extractcmd.mak tools/extractcmd/main.c
	cd tools/extractcmd && $(MAKE) -f extractcmd.mak
tools/extractcmd/extractcmd.mak: tools/extractcmd/extractcmd.pro
	cd tools/extractcmd && qmake $(<F) -o $(@F)

%-sim %-simulator: %-simulator-$(TAG).mak compiler recorder .ALWAYS
	$(MAKE) -f $<
%-fw %-firmware: %-firmware-$(TAG).mak compiler elf2rom .ALWAYS
	$(MAKE) -f $<

%-firmware-$(TAG).mak: %-firmware.pro | compiler elf2rom
	qmake $< CONFIG+=$(CONFIG) -o $@ -spec devices/linux-generic-g++
%-$(TAG).mak: %.pro
	qmake $< CONFIG+=$(CONFIG) -o $@

recorder: recorder/config.h
recorder/config.h: recorder/Makefile
	cd recorder && $(MAKE)

debug-% %-debug:
	$(MAKE) CONFIG=debug $*
release-% %-release:
	$(MAKE) CONFIG=release $*
debug: debug-sim
release: release-all

FONT_LIST=firmware/include/fontlist.h
FONT_BITMAP_LIST=bitmap/fonts/fontlist.h
fonts: bmpfonts ttffonts $(FONT_LIST)
bmpfonts: bmp2font
	cd bitmap/fonts && ./doallfonts.sh
	mv bitmap/fonts/*.c firmware/sys/
$(FONT_BITMAP_LIST): bmpfonts

# Sizes from the base font, and variant fonts we will build
FONT_SIZES=14 18 24 32
FONT_VARIANTS=HelpTitle Help HelpBold HelpItalic HelpCode Menus

FONT_NAMES=$(FONT_SIZES:%=Font%.c) $(FONT_VARIANTS:%=%FontData.c)
FONT_FILES=$(FONT_NAMES:%=firmware/sys/%)
ttffonts: ttf2font $(FONT_FILES)

# Basic font (for stack, etc)
FONT_BASE=fonts/C43SNumericFont.ttf
# Alternate: fonts/adamina/Adamina-Regular.ttf
# Alternate: fonts/C43StandardFont.ttf
firmware/sys/Font%.c: $(FONT_BASE) $(FONT_BITMAP_LIST) $(MAKEFILE_LIST) tools-bin/ttf2font
	tools-bin/ttf2font -t64 -s $* FONT_$* $(FONT_BASE) $@

# Help fonts
FONT_BASE_Help=fonts/PixelOperator.ttf
FONT_SIZE_Help=16
FONT_BASE_HelpTitle=fonts/PixelOperator-Bold.ttf
FONT_SIZE_HelpTitle=24
FONT_BASE_HelpBold=fonts/PixelOperator-Bold.ttf
FONT_SIZE_HelpBold=16
FONT_BASE_HelpItalic=fonts/Gibberesque.ttf
FONT_SIZE_HelpItalic=16
FONT_BASE_HelpCode=fonts/PixelOperatorMono.ttf
FONT_SIZE_HelpCode=16
FONT_BASE_Menus=fonts/PixelOperator.ttf
FONT_SIZE_Menus=16

firmware/sys/%FontData.c: $(FONT_BASE_%) $(MAKEFILE_LIST) tools-bin/ttf2font
	tools-bin/ttf2font -s $(FONT_SIZE_$*) FONT_$* $(FONT_BASE_$*) $@

$(FONT_LIST): $(FONT_BITMAP_LIST)
	tools/build-font-list.sh > $@ $< $(FONT_SIZES) $(FONT_VARIANTS)

FILES=$(shell git ls-files '*.c' '*.h' '*.cpp')
reformat clang-format: $(FILES:%=%.clang-format)
%.clang-format:
	clang-format -i $*

.PRECIOUS: %-firmware-$(TAG).mak %-$(TAG).mak
.ALWAYS:
