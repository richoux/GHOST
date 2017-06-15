# Project name
EXEC=libghost.so

# Compiler
SRCDIR=src src/misc
SRCDIRFLAG=$(foreach sdir, $(SRCDIR), -I$(sdir))
CXXFLAGS=-std=c++14 -fPIC -Ofast -W -Wall -Wextra -pedantic -Wno-sign-compare -Wno-unused-parameter $(SRCDIRFLAG)

# Linker
LFLAGS=-shared $(SRCDIRFLAG)

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
gcc: clean
gcc: CXX=g++
gcc: LINKER=g++ -o
gcc: CXXFLAGS += -DNDEBUG
gcc: $(BINDIR)/$(EXEC)

gcc-debug: clean
gcc-debug: CXX=g++
gcc-debug: LINKER=g++ -o
gcc-debug: CXXFLAGS += -g
gcc-debug: $(BINDIR)/$(EXEC)

clang: clean
clang: CXX=clang++
clang: LINKER=clang++ -o
clang: CXXFLAGS += -DNDEBUG -stdlib=libc++
clang: $(BINDIR)/$(EXEC)

clang-debug: clean
clang-debug: CXX=clang++
clang-debug: LINKER=clang++ -o
clang-debug: CXXFLAGS += -g -stdlib=libc++
clang-debug: $(BINDIR)/$(EXEC)

$(BINDIR)/$(EXEC): $(OBJECTS)
	@$(LINKER) $@ $(LFLAGS) $^

$(OBJDIR)/%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: gcc gcc-debug clang clang-debug clean test travistest doc

clean:
	rm -fr core *~ $(OBJDIR)/*.o $(BINDIR)/$(EXEC) $(SOURCESTILDE)

# test:
# 	$(MAKE)
# 	cd test && $(MAKE) test

test:
	cd test && $(MAKE) test

travistest:
	cd test && $(MAKE) travistest

doc:
	doxygen doc/Doxyfile
