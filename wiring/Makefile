
CC = clang++
CFLAGS = -std=c++14 -Wall
LDFLAGS = -lwiringPi -lpthread
SRC = src
BIN = bin
SOURCES = $(wildcard $(SRC)/*.cpp)
HEADERS = $(wildcard $(SRC)/*.hpp)
EXE = $(BIN)/wiring
CARPUTER_HOST = ford
TMP_DIR_PREFIX = wiringtmp

all : $(BIN) $(EXE)

$(BIN) :
	mkdir -p $(BIN)

$(EXE) : $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $(EXE)

clean :
	rm -f $(EXE)

remote :
	$(eval @_REMOTE_DIR = $(shell ssh $(CARPUTER_HOST) mktemp -d /tmp/$(TMP_DIR_PREFIX).XXXXXXXX))
	@echo "Cleaning up stale temp directories..."
	ssh ford "find /tmp -name '$(TMP_DIR_PREFIX).*' ! -name $(shell basename $(@_REMOTE_DIR)) -type d -exec rm -rf {} + 2>/dev/null || :"
	@echo
	@echo "Copying source to device..."
	scp -r Makefile src/ $(CARPUTER_HOST):$(@_REMOTE_DIR)
	@echo
	@echo "Compiling and running..."
	ssh -t $(CARPUTER_HOST) "cd $(@_REMOTE_DIR) && make && sudo $(EXE)"

.PHONY : clean remote

