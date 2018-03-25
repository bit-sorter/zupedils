CC = gcc
CFLAGS = -Wall -pedantic `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0`
SOURCES = zupedils.c
OBJECTS = $(SOURCES:.c=.o)
EXE = zupedils

$(EXE): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(EXE) $^

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY : clean
clean:
	rm -f *.o $(EXE) core

