# Project name
EXEC=ghost

# Compiler
CXX=g++
IDIR=include include/constraints include/domains include/misc include/objectives include/variables
CXXFLAGS=-std=c++0x -Ofast -W -Wall -Wextra -pedantic -I$(IDIR)

# Linker
LINKER=g++ -o
LFLAGS=-W -Wall -Wextra -pedantic -I$(IDIR)

# Directories
SRCDIR=src src/constraints src/domains src/misc src/objectives src/variables
OBJDIR=obj
BINDIR=bin

# Files
SOURCES=$(foreach sdir, $(SRCDIR), $(wildcard $(sdir)/*.cpp))
OBJECTS=$(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(SOURCES)))

# For rm
SOURCESTILDE=$(foreach sdir, $(SRCDIR), $(wildcard $(sdir)/*.cpp~))
INCLUDESTILDE=$(foreach idir, $(IDIR), $(wildcard $(idir)/*.hpp~))

vpath %.cpp $(SRCDIR)

# Rules
all: CXXFLAGS += -DNDEBUG
all: $(BINDIR)/$(EXEC)

debug: CXXFLAGS += -g
debug: $(BINDIR)/$(EXEC)

$(BINDIR)/$(EXEC): $(OBJECTS)
	@$(LINKER) $@ $(LFLAGS) $(OBJECTS)

$(OBJECTS): $(OBJDIR)/%.o : $(SOURCES)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all debug clean 

clean:
	rm -fr *~ $(OBJECTS) $(BINDIR)/$(EXEC) $(SOURCESTILDE) $(INCLUDESTILDE)

