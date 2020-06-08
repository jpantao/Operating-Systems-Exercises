

OBJ=countNprocMain.o process.o
CFLAGS=-Wall -g

all: countNprocMain


countNprocMain: $(OBJ)
	cc $(CFLAGS) $(OBJ) -o $@

clean:
	rm -f countNprocMain $(OBJ)
