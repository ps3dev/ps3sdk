##
## Configuration
##

CC := gcc
CFLAGS  := -Wall -pedantic -std=c99 -lz -lssl

TARGETS := decrypt-self read-self rebuild-self
OBJECTS :=

##
## Build Rules
##

default: $(TARGETS)

clean:
	rm -f $(TARGETS)

%: %.c $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $< -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

%: %.cpp $(OBJECTS)
	$(CPP) $(CPPFLAGS) $(OBJECTS) $< -o $@

%.o: %.cpp %.h
	$(CPP) $(CPPFLAGS) -c $< -o $@
