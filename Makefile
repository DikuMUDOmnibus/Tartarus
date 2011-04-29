CC = gcc

OBJECTS = main.o commands.o

all: tartarus

%.o: %.c
	$(CC) -Wall -pedantic -ansi -c -g -o $@ $<

tartarus: $(OBJECTS)
	$(CC) -o tartarus $(OBJECTS)

clean:
	@echo Cleaning up.
	rm -f *.o; rm tartarus
	@echo Done.
