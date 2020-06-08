
OBJ=fso-sh.o fs.o disk.o
CFLAGS=-Wall -g

all: fso-sh


fso-sh: fso-sh.o fs.o disk.o
	cc $(CFLAGS) $(OBJ) -o fso-sh

clean:
	rm fso-sh $(OBJ)
