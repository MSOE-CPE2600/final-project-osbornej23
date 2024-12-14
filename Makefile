CC=gcc
CFLAGS=-c -Wall -g 
LDFLAGS=-lpulse -lpulse-simple -lfftw3f -lsndfile -lportaudio -lSDL2 -lSDL2_ttf -lm
SOURCES=equalizer.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=equalizer

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MF $*.d -o $@ $<

# Include dependency files
-include $(SOURCES:.c=.d)

# Clean removes only object and dependency files
clean:
	rm -f $(OBJECTS) *.d


