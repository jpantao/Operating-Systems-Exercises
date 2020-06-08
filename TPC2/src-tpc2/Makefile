
CFLAGS = -Wall -g
LDFLAGS = -lpthread

OBJS = test.o fso_phaser.o pthread_to_pos.o

all:	fso_phase_test

fso_phase_test:	$(OBJS)
	${CC} -o $@ ${OBJS} $(LDFLAGS)


clean:
	rm -f $(OBJS) *~ fso_phase_test
