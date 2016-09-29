
CC = clang++
CFLAGS = -std=c++14 -Wall
LDFLAGS = -lwiringPi -lpthread
SRC = src
BIN = ../bin
SOURCES = $(wildcard $(SRC)/*.cpp)
HEADERS = $(wildcard $(SRC)/*.hpp)
EXE = $(BIN)/wiring

all : $(BIN) $(EXE)

$(BIN) :
	mkdir -p $(BIN)

$(EXE) : $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $(EXE)

.PHONY : clean
clean :
	rm -f $(EXE)

