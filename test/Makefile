#unit tests, run with `make -C test` from the njhcpp root, njhcpp is header only so nothing needs to be built first

ROOT = $(realpath ..)
include $(ROOT)/compfile.mk
include $(ROOT)/makefile-common.mk

COMLIBS += -I$(ROOT)/src -I$(ROOT)/test/catch2

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

$(CATCH_OBJ): catch2/catch_amalgamated.cpp catch2/catch_amalgamated.hpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $(CXXOPT) -I$(ROOT)/test/catch2 -c $< -o $@

clean:
	rm -rf build bin
