EXEC_CSP=tutorial_csp
EXEC_COP=tutorial_cop
EXEC_CFN_SAT=tutorial_cfn_sat
EXEC_CFN_OPT=tutorial_cfn_opt

# Compiler flags
CXXFIRSTFLAGS= -Ofast -DBENCH -W -Wall -Wextra -pedantic -Wno-sign-compare -Wno-unused-parameter
CXXFIRSTFLAGSDEBUG= -g -O0 -DBENCH -W -Wall -Wextra -pedantic -Wno-sign-compare -Wno-unused-parameter 

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	CXX=g++
	CXXFLAGS= -std=c++14 $(CXXFIRSTFLAGS)
	CXXFLAGSDEBUG= -std=c++14 $(CXXFIRSTFLAGSDEBUG)
endif
ifeq ($(UNAME_S),Darwin)
	CXX=clang++
	CXXFLAGS= -std=c++1y  -stdlib=libc++ $(CXXFIRSTFLAGS)
	CXXFLAGSDEBUG= -std=c++1y  -stdlib=libc++ $(CXXFIRSTFLAGSDEBUG)
	LDFLAGS=-pthread -lghost -lc++ -lc++abi
endif

# Directories
SRCDIR=.
OBJDIR=obj
BINDIR=bin
INCLUDEDIR=../include

# Linker flags
LDFLAGS= -pthread -lghost

# Files
#SOURCES=$(foreach sdir, $(SRCDIR), $(wildcard $(sdir)/*.cpp))
#OBJECTS=$(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(SOURCES)))

# For rm
SOURCESTILDE=$(foreach sdir, $(SRCDIR), $(wildcard $(sdir)/*.cpp~))

vpath %.cpp $(SRCDIR)

# Reminder, 'cause it is easy to forget makefile's fucked-up syntax...
# $@ is what triggered the rule, ie the target before :
# $^ is the whole dependencies list, ie everything after :
# $< is the first item in the dependencies list

# Rules
all: $(BINDIR)/$(EXEC_CSP) $(BINDIR)/$(EXEC_COP) $(BINDIR)/$(EXEC_CFN_SAT) $(BINDIR)/$(EXEC_CFN_OPT)

debug: CXXFLAGS= $(CXXFLAGSDEBUG)
debug: $(BINDIR)/$(EXEC_CSP) $(BINDIR)/$(EXEC_COP) $(BINDIR)/$(EXEC_CFN_SAT) $(BINDIR)/$(EXEC_CFN_OPT)

$(BINDIR)/$(EXEC_CSP): $(OBJDIR)/object_data.o $(OBJDIR)/constraint_capacity.o $(OBJDIR)/at_least.o $(OBJDIR)/main.o
	$(CXX) -o $@ $^ $(LDFLAGS)

$(BINDIR)/$(EXEC_COP): $(OBJDIR)/object_data.o $(OBJDIR)/constraint_capacity.o $(OBJDIR)/max_value.o $(OBJDIR)/main_cop.o
	$(CXX) -o $@ $^ $(LDFLAGS)

$(BINDIR)/$(EXEC_CFN_SAT): $(OBJDIR)/object_data.o $(OBJDIR)/constraint_capacity_cfn.o $(OBJDIR)/at_least_cfn.o $(OBJDIR)/main.o
	$(CXX) -o $@ $^ $(LDFLAGS)

$(BINDIR)/$(EXEC_CFN_OPT): $(OBJDIR)/object_data.o $(OBJDIR)/constraint_capacity_cfn.o $(OBJDIR)/max_value.o $(OBJDIR)/main_cop.o
	$(CXX) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/constraint_capacity_cfn.o: $(SRCDIR)/constraint_capacity.cpp
	$(CXX) $(CXXFLAGS) -c -DCFN -I$(INCLUDEDIR) $< -o $@

$(OBJDIR)/at_least_cfn.o: $(SRCDIR)/at_least.cpp
	$(CXX) $(CXXFLAGS) -c -DCFN -I$(INCLUDEDIR) $< -o $@

$(OBJDIR)/main_cop.o: $(SRCDIR)/main.cpp
	$(CXX) $(CXXFLAGS) -c -DOBJECTIVE -I$(INCLUDEDIR) $< -o $@

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -I$(INCLUDEDIR) $< -o $@

.PHONY: clean 

clean:
	rm -fr core *~ $(EXEC) $(OBJDIR)/*.o $(SOURCESTILDE) 
