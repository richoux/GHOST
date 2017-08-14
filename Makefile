# Project name
EXEC=libghost.so

# Compiler
CXX=g++
SRCDIR=src
INCDIR=include include/misc
INCDIRFLAG=$(foreach idir, $(INCDIR), -I$(idir))
CXXFLAGS=-std=c++14 -fPIC -Ofast -W -Wall -Wextra -pedantic -Wno-sign-compare -Wno-unused-parameter $(INCDIRFLAG)

# Linker
LDFLAGS=-shared $(INCDIRFLAG)
#LDFLAGS=-shared $(INCDIRFLAG)

# Directories
OBJDIR=obj
BINDIR=lib

# Files
SOURCES=$(foreach sdir, $(SRCDIR), $(wildcard $(sdir)/*.cpp))
OBJECTS=$(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(SOURCES)))

# For rm
SOURCESTILDE=$(foreach sdir, $(SRCDIR), $(wildcard $(sdir)/*.cpp~))
INCLUDETILDE=$(foreach idir, $(INCDIR), $(wildcard $(idir)/*.hpp~))

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
	$(CXX) -o $@ $(LDFLAGS) $^

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: debug clean test doc install

clean:
	rm -fr core *~ $(OBJDIR)/*.o $(BINDIR)/$(EXEC) $(SOURCESTILDE) $(INCLUDETILDE)

test:
	(cd test && $(MAKE))

doc:
	doxygen doc/Doxyfile

install:
	cp lib/libghost.so /usr/local/lib
	ldconfig
	rm -fr /usr/local/include/ghost
	cp -r include /usr/local/include/ghost
