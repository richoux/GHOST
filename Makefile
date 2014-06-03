# Project name
EXEC=ghost

# Compiler
CXX=g++
IDIR=include include/constraints include/domains include/misc include/objectives include/variables
IDIRFLAG=$(foreach idir, $(IDIR), -I$(idir))
CXXFLAGS=-std=c++0x -Ofast -W -Wall -Wextra -pedantic -Wno-sign-compare $(IDIRFLAG)

# Linker
LINKER=g++ -o
LFLAGS=$(IDIRFLAG)

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

# Reminder, 'cause it is easy to forget makefile's fucked-up syntax...
# $@ is what triggered the rule, ie the target before :
# $^ is the whole dependencies list, ie everything after :
# $< is the first item in the dependencies list

# Rules
all: CXXFLAGS += -DNDEBUG
all: $(BINDIR)/$(EXEC)

debug: CXXFLAGS += -g
debug: $(BINDIR)/$(EXEC)

$(BINDIR)/$(EXEC): $(OBJECTS)
	@$(LINKER) $@ $(LFLAGS) $^

$(OBJDIR)/%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all debug clean 

clean:
	rm -fr core *~ $(OBJECTS) $(BINDIR)/$(EXEC) $(SOURCESTILDE) $(INCLUDESTILDE)

