# expects
# GXX_PATH -> path to g++ executable
# BUILD_DIR -> path to build directory

CXX=$(GXX_PATH)
BIN = $(BUILD_DIR)/blblstd.o

MAIN = src/blblstd.cpp
SRC = src/blblstd.cpp
SRC += src/blblstd.hpp
SRC += src/arena.cpp
SRC += src/link_list.cpp
SRC += src/list.cpp
SRC += src/memory.cpp
SRC += src/utils.cpp
SRC += src/virtual_arena.cpp
SRC += src/virtual_memory.cpp

INC = .
INC += src

LIB = .
CFLAGS = -g3 -std=c++20


bin: $(BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN): $(BUILD_DIR) $(SRC)
	$(CXX) $(CFLAGS) -c $(MAIN) $(INC:%=-I%) $(LIB:%=-L%) $(LDFLAGS) -o $@

clean:
	rm -rf $(BUILD_DIR)

re: clean bin

.PHONY: bin clean re
