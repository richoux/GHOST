EXEC_CSP=tutorial_csp
EXEC_COP=tutorial_cop
EXEC_CFN_SAT=tutorial_cfn_sat
EXEC_CFN_OPT=tutorial_cfn_opt

# Compiler flags
CXXFIRSTFLAGS= -O3 -DBENCH -W -Wall -Wextra -pedantic -Wno-sign-compare -Wno-unused-parameter
CXXFIRSTFLAGSDEBUG= -g -O0 -DBENCH -W -Wall -Wextra -pedantic -Wno-sign-compare -Wno-unused-parameter -fsanitize=address

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	CXX=g++
	CXXFLAGS= -std=c++17 $(CXXFIRSTFLAGS)
	CXXFLAGSDEBUG= -std=c++17 $(CXXFIRSTFLAGSDEBUG)
endif
ifeq ($(UNAME_S),Darwin)
	CXX=clang++
	CXXFLAGS= -std=c++1z  -stdlib=libc++ $(CXXFIRSTFLAGS)
	CXXFLAGSDEBUG= -std=c++1z  -stdlib=libc++ $(CXXFIRSTFLAGSDEBUG)
	LDFLAGS=-pthread -lghost -lc++ -lc++abi
	LDFLAGSSTATIC= -lghost_static -lc++ -lc++abi
endif

# Directories
SRCDIR=.
OBJDIR=obj
BINDIR=bin
DYNDIR=dynamic
STATDIR=static

# Linker flags
LDFLAGS= -pthread -lghost
LDFLAGSSTATIC= -lghost_static

LDFLAGSDEBUG= -pthread -lghostd -fsanitize=address
LDFLAGSSTATICDEBUG= -lghost_staticd -fsanitize=address

# For rm
SOURCESTILDE=$(foreach sdir, $(SRCDIR), $(wildcard $(sdir)/*.cpp~))

vpath %.cpp $(SRCDIR)

EXEC= $(BINDIR)/$(DYNDIR)/$(EXEC_CSP) $(BINDIR)/$(DYNDIR)/$(EXEC_COP) $(BINDIR)/$(DYNDIR)/$(EXEC_CFN_SAT) $(BINDIR)/$(DYNDIR)/$(EXEC_CFN_OPT) $(BINDIR)/$(STATDIR)/$(EXEC_CSP) $(BINDIR)/$(STATDIR)/$(EXEC_COP) $(BINDIR)/$(STATDIR)/$(EXEC_CFN_SAT) $(BINDIR)/$(STATDIR)/$(EXEC_CFN_OPT)

# Reminder, 'cause it is easy to forget makefile's fucked-up syntax...
# $@ is what triggered the rule, ie the target before :
# $^ is the whole dependencies list, ie everything after :
# $< is the first item in the dependencies list

# Rules
all: $(EXEC)

debug: CXXFLAGS= $(CXXFLAGSDEBUG)
debug: LDFLAGS= $(LDFLAGSDEBUG)
debug: LDFLAGSSTATIC= $(LDFLAGSSTATICDEBUG)
debug: $(EXEC)

$(BINDIR)/$(DYNDIR)/$(EXEC_CSP): $(OBJDIR)/object_data.o $(OBJDIR)/constraint_capacity.o $(OBJDIR)/at_least.o $(OBJDIR)/main.o
	$(CXX) -o $@ $^ $(LDFLAGS)

$(BINDIR)/$(DYNDIR)/$(EXEC_COP): $(OBJDIR)/object_data.o $(OBJDIR)/constraint_capacity.o $(OBJDIR)/max_value.o $(OBJDIR)/main_cop.o
	$(CXX) -o $@ $^ $(LDFLAGS)

$(BINDIR)/$(DYNDIR)/$(EXEC_CFN_SAT): $(OBJDIR)/object_data.o $(OBJDIR)/constraint_capacity_cfn.o $(OBJDIR)/at_least_cfn.o $(OBJDIR)/main.o
	$(CXX) -o $@ $^ $(LDFLAGS)

$(BINDIR)/$(DYNDIR)/$(EXEC_CFN_OPT): $(OBJDIR)/object_data.o $(OBJDIR)/constraint_capacity_cfn.o $(OBJDIR)/max_value.o $(OBJDIR)/main_cop.o
	$(CXX) -o $@ $^ $(LDFLAGS)

$(BINDIR)/$(STATDIR)/$(EXEC_CSP): $(OBJDIR)/object_data.o $(OBJDIR)/constraint_capacity.o $(OBJDIR)/at_least.o $(OBJDIR)/main.o
	$(CXX) -o $@ $^ $(LDFLAGSSTATIC)

$(BINDIR)/$(STATDIR)/$(EXEC_COP): $(OBJDIR)/object_data.o $(OBJDIR)/constraint_capacity.o $(OBJDIR)/max_value.o $(OBJDIR)/main_cop.o
	$(CXX) -o $@ $^ $(LDFLAGSSTATIC)

$(BINDIR)/$(STATDIR)/$(EXEC_CFN_SAT): $(OBJDIR)/object_data.o $(OBJDIR)/constraint_capacity_cfn.o $(OBJDIR)/at_least_cfn.o $(OBJDIR)/main.o
	$(CXX) -o $@ $^ $(LDFLAGSSTATIC)

$(BINDIR)/$(STATDIR)/$(EXEC_CFN_OPT): $(OBJDIR)/object_data.o $(OBJDIR)/constraint_capacity_cfn.o $(OBJDIR)/max_value.o $(OBJDIR)/main_cop.o
	$(CXX) -o $@ $^ $(LDFLAGSSTATIC)

$(OBJDIR)/constraint_capacity_cfn.o: $(SRCDIR)/constraint_capacity.cpp
	$(CXX) $(CXXFLAGS) -c -DCFN $< -o $@

$(OBJDIR)/at_least_cfn.o: $(SRCDIR)/at_least.cpp
	$(CXX) $(CXXFLAGS) -c -DCFN $< -o $@

$(OBJDIR)/main_cop.o: $(SRCDIR)/main.cpp
	$(CXX) $(CXXFLAGS) -c -DOBJECTIVE $< -o $@

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean 

clean:
	rm -fr core *~ $(EXEC) $(OBJDIR)/*.o $(SOURCESTILDE) 
