# D�finition des variables pour les r�gles implicites.
# compilateur C
CC=gcc

CFLAGS= -g -Wall -Wextra -std=c99
LDFLAGS= -g -lm

#headers
INC = constante.h decodage.h algo.h

#sources
SRC:= decodage.c algo.c  DVB_RCS.c

EXE:= DVB

OBJ = $(SRC:.c=.o)

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) -o $@ $(LDFLAGS) $^

%.o: %.c $(INC)
	$(CC) -c $(CFLAGS) $*.c

run: $(EXE)
	@./$^


# faire le m�nage
clean:
	rm -f *.o core* *~ $(EXE)



