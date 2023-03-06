CXX=/c/msys64/mingw64/bin/g++.exe

SRC = test.cpp
INC = .
LIB = .
CFLAGS = -g3 -std=c++20

EXECUTABLE = $(BUILD_DIR)/test.exe

exe: $(BUILD_DIR) $(EXECUTABLE)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(EXECUTABLE): $(SRC)
	$(CXX) $(CFLAGS) $^ $(INC:%=-I%) $(LIB:%=-L%) $(LDFLAGS) -o $@

.PHONY: exe
