# Compiler
CC = gcc
CFLAGS = -Wall -I./include  # Includes the header directory

# Source files and objects
SRC = src/main.c  # Source files
OBJ = src/main.o  # Object files generated from source files

# Output executable
OUT = ./bin/nnsr

# Libraries needed
LIBS = -lfftw3 -lm -ljpeg -lnn

# Default rule (build everything)
all: $(OUT)

# Rule to compile the executable
$(OUT): $(OBJ)
	$(CC) $(OBJ) -o $(OUT) $(LIBS)

# Rule to compile .c files into .o files
$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

# Clean generated files
clean:
	rm -f $(OBJ) $(OUT)

