

SRC = $(shell echo *.c)

OBJ=$(SRC:%.c=%.o)


all: $(OBJ)

clean:
	-@rm -f *.o
	-@rm -f *.a

%.o : %.c
	@$(CC) ${ARM_ELF_CFLAGS} -c $< -o $@

