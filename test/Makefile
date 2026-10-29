#unit tests, run with `make -C test` from the njhcpp root, njhcpp is header only so nothing needs to be built first

ROOT = $(realpath ..)
include $(ROOT)/compfile.mk
include $(ROOT)/makefile-common.mk

COMLIBS += -I$(ROOT)/src

#Catch2 comes from the catch2 package's include path in makefile-common.mk, its amalgamated source is compiled here
CATCH_DIR = $(firstword $(foreach d,$(patsubst -I%,%,$(filter -I%,$(COMLIBS))),$(if $(wildcard $(d)/catch_amalgamated.cpp),$(d))))
ifeq (,$(CATCH_DIR))
$(error catch_amalgamated.cpp not found in COMLIBS, re-run ./configure.py and ./setup.py so catch2 is in makefile-common.mk)
endif

TEST_SRC = $(wildcard unit/*.cpp)
TEST_OBJ = $(patsubst unit/%.cpp, build/unit/%.o, $(TEST_SRC))
CATCH_OBJ = build/catch_amalgamated.o
TEST_BIN = bin/njhcppUnitTests

.PHONY: all run clean

all: run

run: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_OBJ) $(CATCH_OBJ)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(CXXOPT) -o $@ $^ $(LD_FLAGS)

build/unit/%.o: unit/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CXXOPT) $(COMLIBS) -c $< -o $@

$(CATCH_OBJ): $(CATCH_DIR)/catch_amalgamated.cpp $(CATCH_DIR)/catch_amalgamated.hpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(CXXOPT) -I$(CATCH_DIR) -c $< -o $@

clean:
	rm -rf build bin
