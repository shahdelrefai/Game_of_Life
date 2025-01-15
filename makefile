# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -pthread
INCLUDE_DIR = /opt/homebrew/Cellar/armadillo/14.2.2/include
LIBRARY_DIR = /opt/homebrew/Cellar/armadillo/14.2.2/lib
LIBS = -larmadillo

# Source and target files
SRC = GameOfLife.cpp
OBJ = gol.o
TARGET = gol

# Default target
all: $(TARGET)

# Rule to compile the source file into object file
$(OBJ): $(SRC)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $(SRC) -o $(OBJ)

# Rule to link the object file and create the executable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -L$(LIBRARY_DIR) $(LIBS) -o $(TARGET)

# Clean the build files
clean:
	rm -f $(OBJ) $(TARGET)

# Rule to remove all build files (including dependencies)
fclean: clean

# Rebuild everything from scratch
re: fclean all
