CC = gcc

jsdir = ./jansson

OBJECTS = area.o commands.o comm.o events.o game_object.o main.o npc.o player.o shared.o
JANSSON_OBJS = $(jsdir)/dump.o $(jsdir)/error.o $(jsdir)/hashtable.o $(jsdir)/load.o \
               $(jsdir)/memory.o $(jsdir)/pack_unpack.o $(jsdir)/strbuffer.o \
               $(jsdir)/utf.o $(jsdir)/value.o

TARGETS = jansson/libjansson.a tartarus

all: $(TARGETS)

%.o: %.c
	$(CC) -Wall -std=gnu99 -c -g -o $@ $<

jansson/libjansson.a: $(JANSSON_OBJS)
	ar r jansson/libjansson.a $(JANSSON_OBJS)
	ranlib jansson/libjansson.a

tartarus: $(OBJECTS)
	$(CC) -o tartarus $(OBJECTS) jansson/libjansson.a -levent

clean:
	@echo Cleaning up.
	rm -f *.o; rm tartarus; rm -f jansson/*.o; rm jansson/libjansson.a;
	@echo Done.
