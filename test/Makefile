# Directories
SRCDIR=src
IDIR=include
OBJDIR=obj
BINDIR=bin

# Compiler flags
IDIRFLAGS=$(foreach idir, $(IDIR), -I$(idir))
CXXFIRSTFLAGS= -O2 -DNDEBUG -W -Wall -Wextra -pedantic -Wno-sign-compare -Wno-unused-parameter $(IDIRFLAGS)

# Compiler
ifeq (,$(findstring g++,$(CXX)))
	CXXFLAGS= -std=c++14 $(CXXFIRSTFLAGS)
else
	CXXFLAGS= -std=c++1y -stdlib=libc++ $(CXXFIRSTFLAGS)
endif


# Linker flags
LDFLAGS=-Llib -pthread -lgtest -lgmock -lghost

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
all: $(EXECS)

$(BINDIR)/test_domain: $(OBJDIR)/test_domain.o
	$(CXX) -o $@ $^ $(LDFLAGS)

$(BINDIR)/test_variable: $(OBJDIR)/test_variable.o
	$(CXX) -o $@ $^ $(LDFLAGS)


$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean 

clean:
	rm -fr core *~ $(EXECS) $(OBJECTS) $(SOURCESTILDE)
