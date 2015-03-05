
# Main target of this makefile
TARGET=ggl
LIBTARGET= lib$(TARGET).a
# Modules (ie subdirectories) used for the target
MODULES = ggl



all: allfiles

install:
	cp $(LIBTARGET) $(LIBS_PATH)

allfiles: $(LIBTARGET)



clean:
	@for I in ${MODULES}; do cd $$I && $(MAKE) $@ && cd ..; done;
	-@rm -f $(LIBTARGET)
	-@rm -rf doc/


$(LIBTARGET) : */*.c 
	@for I in ${MODULES}; do cd $$I && $(MAKE) && cd ..; done;
	$(AR) rcv $(LIBTARGET) */*.o

doc:
	doxygen

