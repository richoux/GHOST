# Project name
EXEC=libghost.so

# Compiler
CXX=g++
SRCDIR=src src/misc
SRCDIRFLAG=$(foreach sdir, $(SRCDIR), -I$(sdir))
CXXFLAGS=-std=c++1y -fPIC -Ofast -W -Wall -Wextra -pedantic -Wno-sign-compare -Wno-unused-parameter $(SRCDIRFLAG)

# Linker
LDFLAGS=-shared $(SRCDIRFLAG)

# Directories
OBJDIR=obj
BINDIR=bin

# Files
SOURCES=$(foreach sdir, $(SRCDIR), $(wildcard $(sdir)/*.cpp))
OBJECTS=$(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(SOURCES)))

# For rm
SOURCESTILDE=$(foreach sdir, $(SRCDIR), $(wildcard $(sdir)/*.cpp~))

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

$(BINDIR)/$(EXEC): $(OBJDIR)/domain.o $(OBJDIR)/variable.o #$(OBJECTS)
	@$(CXX) -o $@ $(LDFLAGS) $^

$(OBJDIR)/%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: debug clean test doc

clean:
	@rm -fr core *~ $(OBJDIR)/*.o $(BINDIR)/$(EXEC) $(SOURCESTILDE)

test:
	@(cd test && $(MAKE))

doc:
	@doxygen doc/Doxyfile
