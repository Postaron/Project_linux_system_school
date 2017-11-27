CC=gcc
CFLAGS=-pipe -ansi -W -Wall -Wextra -pedantic -fmessage-length=0
LDFLAGS=-pipe
SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)
DEP=$(wildcard *.h)
DEBUG=
EXEC=projet

ifeq ($(DEBUG), yes)
	CFLAGS += -g3
else
	CFLAGS += -O3
endif

all: $(EXEC)

$(EXEC):	$(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

main.o:	$(DEP)

%.o:	%.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean mrproper
clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)
