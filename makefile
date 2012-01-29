CC = g++
EIGEN_OPT = -DNDEBUG
O = 3
OPTIMIZE_OPT = $(O)
OPT = -O$(OPTIMIZE_OPT) -Wall $(EIGEN_OPT)

INCLUDES = -I src -I lib
INCLUDES_T = -I lib/gtest/include  -I lib/gtest
CFLAGS = $(OPT) $(INCLUDES)
ifneq (,$(filter g++%,$(CC)))
	CFLAGS += -std=gnu++0x
endif
CFLAGS_T = $(CFLAGS) $(INCLUDES_T) -DGTEST_HAS_PTHREAD=0 

## --- multiple platform section ---
UNAME := $(shell uname)      # uname provides information about the platform
ifneq (, $(filter CYGWIN%,$(UNAME))) # Windows under Cygwin
	ifneq (,$(filter g++%,$(CC)))
		CFLAGS += -static-libgcc -static-libstdc++
	endif	
else ifeq (Darwin, $(UNAME)) # Mac OS X
	OPT += -g
else ifeq (LINUX, $(UNAME))
	OPT += -g
	CFLAGS_T += -lpthread
else # assume Linux
	OPT += -g
	CFLAGS_T += -lpthread
endif
## --------------------------------

# find all unit tests
UNIT_TESTS := $(shell find src/test/ -type f -name '*_test.cpp')
UNIT_TESTS_DIR := $(sort $(dir $(UNIT_TESTS)))
UNIT_TESTS_OBJ := $(UNIT_TESTS:src/test/%_test.cpp=test/%)

# DEFAULT
# =========================================================

.PHONY: all test-all test-unit
all: test-all

# TEST
# =========================================================

test:
	mkdir -p ar test 
	$(foreach var,$(UNIT_TESTS_DIR:src/%/=%), mkdir -p $(var);)

ar/libgtest.a:  | test
	$(CC) $(CFLAGS_T) -c lib/gtest/src/gtest-all.cc -o ar/gtest-all.o
	ar -rv ar/libgtest.a ar/gtest-all.o


# The last argument, $$(wildcard src/stan/$$(dir $$*)*.hpp), puts *.hpp files from the
#   same directory as a prerequisite. For example, for test/prob/distributions, it will expand to
#   all the hpp files in the src/stan/prob/ directory.
.SECONDEXPANSION:
test/% : src/test/%_test.cpp ar/libgtest.a $$(wildcard src/stan/$$(dir $$*)*.hpp)
	@echo '================================================================================'
	@echo '================================================================================'
	$(CC) $(CFLAGS_T) src/$@_test.cpp lib/gtest/src/gtest_main.cc ar/libgtest.a -o $@
	$@ --gtest_output="xml:$@.xml"

# run all tests
test-all: $(UNIT_TESTS_OBJ) #demo/gm
	$(foreach var,$(UNIT_TESTS_OBJ), $(var) --gtest_output="xml:$(var).xml";)

# run unit tests without having make fail
test-all-no-fail: $(UNIT_TESTS_OBJ) #demo/gm
	-$(foreach var,$(UNIT_TESTS_OBJ), $(var) --gtest_output="xml:$(var).xml";)

## Attempt to build all unit tests as one.
#$(CC) $(CFLAGS_T) $(UNIT_TESTS) lib/gtest/src/gtest_main.cc ar/libgtest.a -o test/unit
#-test/unit --gtest_output="xml:test/unit.xml";)

# MODELS (to be passed through demo/gm)
# =========================================================
# find all bugs models
BUGS_MODELS = $(subst src/,,$(wildcard src/models/bugs_examples/vol*/*/*.stan))

.PHONY: test-models
test-models: $(BUGS_MODELS) | demo/gm models

models:
	mkdir -p models

models/%.stan : | demo/gm models
	mkdir -p $(dir $@)
	@echo '--- translating Stan graphical model to C++ code ---'
	cat src/$@ | demo/gm > $(basename $@).cpp
	@echo '--- compiling C++ model ---'
	$(CC) $(CFLAGS) $(basename $@).cpp -o $(basename $@)
	@echo '--- run the model ---'
	$(basename $@) --data=src/$(basename $@).Rdata --samples=$(basename $@).csv


# DEMO
# =========================================================

demo:
	mkdir -p demo

demo/% : src/demo/%.cpp | demo
	$(CC) $(CFLAGS) src/$@.cpp -c -o $@.o
	$(CC) $(CFLAGS) $@.o -o $@

demo-all: demo/bivar_norm demo/model1 demo/eight_schools


# DOC
# =========================================================

.PHONY: dox doxygen
dox:
	mkdir -p doc/api

doxygen: | dox
	doxygen doc/doxygen.cfg


# CLEAN
# =========================================================

.PHONY: clean clean-dox clean-all
clean:
	rm -rf demo test *.dSYM

clean-models:
	rm -rf models

clean-dox:
	rm -rf doc/api

clean-all: clean clean-dox clean-models
	rm -rf ar
