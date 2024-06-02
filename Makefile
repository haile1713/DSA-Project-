<<<<<<< HEAD
CC = gcc

# Compiler flags shows more errors
CFLAGS = -Wall -Wextra -pedantic -std=c99   

# output Executable name
EXEC = texty

# Source files
SRC = main.c editor.c input.c output.c fileio.c row.c find.c

# Object files change .c to .o
OBJ = $(SRC:.c=.o)

# Default target 
all: $(EXEC)

# Link object files to create the executable
$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(OBJ) $(EXEC)

# Phony targets
.PHONY: all clean
