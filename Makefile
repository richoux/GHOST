# Project name
# EXEC=libghost.so

# Compiler
ifeq ($(CXX),)
  CXX=g++
endif
SRCDIR=src
INCDIR=include include/misc
INCDIRFLAG=$(foreach idir, $(INCDIR), -I$(idir))
CXXFLAGS=-std=c++14 -fPIC -W -Wall -Wextra -pedantic -Wno-sign-compare -Wno-unused-parameter $(INCDIRFLAG)

# Linker
# LDFLAGS=-shared $(INCDIRFLAG)

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

all: CXXFLAGS += -DNDEBUG -Ofast 
all: $(OBJECTS)
	ar cru $(BINDIR)/libghost.a $(OBJECTS)
#all: $(BINDIR)/$(EXEC)	

debug: CXXFLAGS += -DDEBUG -g -O0
debug: $(OBJECTS)
	ar cru $(BINDIR)/libghost.a $(OBJECTS)
#debug: $(BINDIR)/$(EXEC)	

#$(BINDIR)/$(EXEC): $(OBJECTS)
#	$(CXX) -o $@ $(LDFLAGS) $^

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
	cp lib/libghost.a /usr/local/lib
	ldconfig
	rm -fr /usr/local/include/ghost
	cp -r include /usr/local/include/ghost
