CC = mpicc
CFLAGS = -Wall -g -Wpointer-arith -Wcast-align   -Wunreachable-code
PROG = timegen
LIBS = -pthread -lgsl -lgslcblas -lm

COMPILE = $(CC) $(CFLAGS) -c
OBJFILES := $(patsubst %.c,%.o,$(wildcard *.c))

all: $(PROG)

$(PROG): $(OBJFILES)
	$(CC)  -g -o $(PROG) $(OBJFILES) $(LIBS)
%.o: %.c
	$(COMPILE)  -o $@ $<
clean:
	rm -f $(PROG)
	rm -f *.o 
check-syntax :
	gcc  -Wall -o /dev/null -S ${CHK_SOURCES}

