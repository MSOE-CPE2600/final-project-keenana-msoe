CC=gcc
CFLAGS=-c -Wall
LDFLAGS= 
LDFLAGSC= 
EXECUTABLES=controller backend

# Sources for each executable
CONTROLLER_SOURCE=controller.c
BACKEND_OBJECTS=backend.o thermostat.o

all: $(EXECUTABLES)

# Build the 'controller' executable
controller: $(CONTROLLER_SOURCE)
	$(CC) -o $@ $< $(LDFLAGSC)

# Build the 'backend' executable
backend: $(BACKEND_OBJECTS)
	$(CC) -o $@ $(BACKEND_OBJECTS) $(LDFLAGS)

# Rule to compile .c files into .o
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(EXECUTABLES) *.o
