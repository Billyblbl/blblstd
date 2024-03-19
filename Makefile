# expects
# GXX_PATH -> path to g++ executable
# BUILD_DIR -> path to build directory

CXX=$(CXX_PATH)
BIN = $(BUILD_DIR)/blblstd.o

MAIN = src/blblstd.cpp
SRC = $(MAIN)
SRC += src/blblstd.hpp
SRC += src/arena.cpp
SRC += src/link_list.cpp
SRC += src/list.cpp
SRC += src/memory.cpp
SRC += src/scratch.cpp
SRC += src/utils.cpp
SRC += src/virtual_memory.cpp
SRC += src/module.cpp
SRC += src/high_order.cpp

INC = .
INC += src

LIB = .
CXXFLAGS += -std=c++23
# CFLAGS += -g3
CXXFLAGS += -fno-exceptions

COLOR=\033[0;34m
NOCOLOR=\033[0m

bin: $(BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN): $(BUILD_DIR) $(SRC)
	@echo -e "Building $(COLOR)blblstd$(NOCOLOR)"
	@$(CXX) $(CXXFLAGS) -c $(MAIN) $(INC:%=-I%) $(LIB:%=-L%) $(LDFLAGS) -o $@

clean:
	rm -rf $(BUILD_DIR)

test: $(BIN)
	@echo -e "Building $(COLOR)test$(NOCOLOR)"
	@$(CXX) test.cpp $(BIN) $(CXXFLAGS) -o $(BUILD_DIR)/test.exe $(INC:%=-I%) $(LIB:%=-L%) $(LDFLAGS)
	@echo -e "Testing $(COLOR)blblstd$(NOCOLOR)"
	$(BUILD_DIR)/test.exe

re: clean bin

.PHONY: bin clean test re
