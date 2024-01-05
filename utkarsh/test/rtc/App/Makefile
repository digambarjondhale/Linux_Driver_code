CC = gcc

SRC_DIR = src

BIN_DIR = bin

EXE = main

SRCS = $(wildcard $(SRC_DIR)/*.c)

all: $(BIN_DIR)

$(BIN_DIR): $(SRCS)
	mkdir -p $(BIN_DIR)

	$(CC) -o $(BIN_DIR)/$(EXE) $(SRCS)

clean:
	rm -rf $(BIN_DIR)
