# OS specific
ifeq ($(OS),Windows_NT)
    CFLAGS += -D WIN32
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CFLAGS += -D LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
        CFLAGS += -D OSX
		LDFLAGS += -framework CoreServices -framework CoreFoundation -framework AudioUnit \
				   -framework AudioToolbox -framework CoreAudio
    endif
endif

CC=g++
LD=$(CC)
CFLAGS += -I. -I.. -std=c++17 `pkg-config --cflags libgme` -I/opt/homebrew/include -I./argspp/src -g -DDEBUG -Wall -Wextra
LIBS =	`pkg-config --libs libgme` -L/opt/homebrew/lib -lportaudio
MACHINE = $(shell $(CC) -dumpmachine)
EXE = gmu-$(MACHINE)

#all: debug
#.PHONY: debug release

#debug: CFLAGS += -g -DDEBUG -O1 -Wall -Wextra
#debug: $(EXE)

#release: CFLAGS += -O3 -DNDEBUG
#release: $(EXE)

# Files
SRC_DIR := src
OBJ_DIR := obj/$(MACHINE)
COMMON_SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SOURCES))

# Rules
$(EXE): $(OBJECTS) $(OBJ_DIR)/args.o
	@echo Linking $(EXE)...
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo Compiling $(@)...
	@mkdir -p $(OBJ_DIR)
	$(CC) $(LDFLAGS) $(CFLAGS) -c $< -o $@ $(LIBS)

$(OBJ_DIR)/args.o: argspp/src/args.cpp
	@echo Compiling argspp.o...
	@mkdir -p $(@D)
	$(MAKE) -C argspp/ lib
	cp argspp/bin/args.o $(OBJ_DIR)

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(EXE)
	cd argspp/ && make clean
