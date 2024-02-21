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

CC ?= g++
LD = $(CC)
CFLAGS += -I. -I.. -std=c++14 `pkg-config --cflags libgme` -I/opt/homebrew/include
LIBS =	`pkg-config --libs libgme` -L/opt/homebrew/lib -lportaudio
MACHINE = $(shell $(CC) -dumpmachine)
EXE = gmu-$(MACHINE)

# main objects
OBJS = main.o Player.o PortAudioSoundDriver.o

MACHINE_OBJS = $(OBJS:%.o=obj/$(MACHINE)/%.o)

all: release
.PHONY: debug release

debug: CFLAGS += -g -DDEBUG -O1 -Wall -Wextra
debug: $(EXE)

release: CFLAGS += -O3 -DNDEBUG
release: $(EXE)

$(EXE): src/main.cpp src/Player.cpp src/PortAudioSoundDriver.cpp
	@echo Compiling $(EXE)...
	@mkdir -p $(@D)
	g++ $(LDFLAGS) $(CFLAGS) src/main.cpp src/Player.cpp src/PortAudioSoundDriver.cpp -o $(EXE) $(LIBS)

clean:
	rm -f $(MACHINE_OBJS) $(EXE)
	rm -r obj/$(MACHINE)
	rm -d obj/ 2>/dev/null
