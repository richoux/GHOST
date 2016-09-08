# Directories
SRCDIR=src
IDIR=include ../include
OBJDIR=obj
BINDIR=bin
LIBDIR=lib

# Compiler flags
IDIRFLAGS=$(foreach idir, $(IDIR), -I$(idir))
CXXFLAGS=-std=c++14 -Ofast -W -Wall -Wextra -pedantic -Wno-sign-compare -Wno-unused-parameter $(IDIRFLAGS)

# Linker flags
LFLAGS=-pthread

# Files
SOURCES=$(foreach sdir, $(SRCDIR), $(wildcard $(sdir)/*.cpp))
OBJECTS=$(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(SOURCES)))

# Binaries name
EXECS=$(patsubst %.cpp, $(BINDIR)/%, $(notdir $(SOURCES)))

# For rm
SOURCESTILDE=$(foreach sdir, $(SRCDIR), $(wildcard $(sdir)/*.cpp~))

vpath %.cpp $(SRCDIR)

# Reminder, 'cause it is easy to forget makefile's fucked-up syntax...
# $@ is what triggered the rule, ie the target before :
# $^ is the whole dependencies list, ie everything after :
# $< is the first item in the dependencies list

# Rules
test: clean
test: CXX=g++
test: LINKER=g++ -o
test: CXXFLAGS += -DNDEBUG
test: LIBGTEST=$(LIBDIR)/libgtest.a
test: $(EXECS)

# travistest: CXX=g++
travistest: LINKER=g++ -o
travistest: CXXFLAGS += -DNDEBUG
travistest: LIBGTEST=/usr/local/lib/libgtest.a
travistest: IDIRFLAGS=-I../include
travistest: $(EXECS)

$(EXECS): $(OBJECTS)
	@$(LINKER) $@ $(LFLAGS) $^ $(LIBGTEST)

$(OBJDIR)/%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: test travistest clean 

clean:
	rm -fr core *~ $(EXECS) $(OBJECTS) $(SOURCESTILDE)
