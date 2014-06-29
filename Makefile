EXECUTABLE=terracer
EXTRALIBS=-lSDL2_image -lm -lSDL2_ttf -lSDL2_mixer
#Uncomment the following line to suppress the gcc commandline
QUIET=@
#Suggested Optimization Levels: -O0 (fastest), -Og (good middleground), -O2 (slowest compile)
OPTIMIZATION=-O0

#SOURCES=src/main.c $(wildcard src/*.c)
SOURCES=entity.cpp player.cpp main.cpp level.cpp util.cpp common.cpp client.cpp connection.cpp server.cpp
EXTERNALSOURCES=jsoncpp.cpp

CC=ccache g++
C99MODE=-std=c++0x
EXTRACFLAGS=-g $(C99MODE) -Wuninitialized -Wmissing-field-initializers $(OPTIMIZATION)

debug: EXTRACFLAGS +=-DDEBUG -g
warn: EXTRACFLAGS += -Wall -Wextra

# Targetting Windows x64
win32: CC:=x86_64-w64-mingw32-g++
win32: EXECUTABLE:=$(EXECUTABLE).exe
win32: CFLAGS=-I$(WINFOLDER)include/SDL2 -Dmain=SDL_main $(EXTRACFLAGS) -Iexternals/include
win32: LIBS=-L$(WINFOLDER)lib -lmingw32 -lSDL2main -lSDL2 $(EXTRALIBS) -lmswsock -lws2_32  $(BOOSTDIR)/lib/libboost_thread-mgw48-mt-1_55.a $(BOOSTDIR)/lib/libboost_system-mgw48-mt-1_55.a


ifeq ($(OS),Windows_NT)
# Building on Windows
WINFOLDER:=C:/mingw64/x86_64-w64-mingw32/
BOOSTDIR=C:/mingw64/boostgcc
EXTRACFLAGS+= -I$(BOOSTDIR)/include -L$(BOOSTDIR)/lib

all: win32
win32: compile
run: win32
	@echo
	./$(EXECUTABLE).exe
gdb: debug
	gdb --eval-command=run --args ./$(EXECUTABLE)
else
# Building on Linux
WINFOLDER:=/usr/x86_64-w64-mingw32/
CFLAGS=$(shell sdl2-config --cflags) $(EXTRACFLAGS) -Iexternals/include -march=native
LIBS=$(shell sdl2-config --libs) $(EXTRALIBS) -lpthread -lboost_thread -lboost_system

all: clean compile
win32: all
	zip $(EXECUTABLE).zip $(EXECUTABLE)
	upload $(EXECUTABLE).zip
run: all
	@echo
	./$(EXECUTABLE) --software
gdb: debug
	gdb --eval-command=run --args ./$(EXECUTABLE) --software
endif

OBJECTS=$(patsubst %.cpp,build/%.o, $(SOURCES))
EXTERNALOBJECTS=$(patsubst %.cpp,externals/build/%.o, $(EXTERNALSOURCES))


warn: all
ifeq ($(MAKECMDGOALS),debug)
debug: clean run clean2
else
debug: clean all clean2
endif


$(OBJECTS): build/%.o : src/%.cpp
	@echo Compiling $<...
	$(QUIET)$(CC) $(CFLAGS) -o $@ -c $<
$(EXTERNALOBJECTS): externals/build/%.o : externals/src/%.cpp
	$(CC) -Iexternals/include -o $@ -c $<
clean:
	-rm -f build/*.o
clean2: #A second one so we can execute clean twice
	-rm -f build/*.o
cleanall:
	-rm -f build/*.o
	-rm -f externals/build/*.o


compile: $(OBJECTS) $(EXTERNALOBJECTS)
	@echo
	@echo Linking $(EXECUTABLE)...
	$(QUIET)$(CC) $(CFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(EXTERNALOBJECTS) $(LIBS)


zip: win32
	zip -r -u $(EXECUTABLE).zip $(EXECUTABLE).exe $(EXECUTABLE).bat $(EXECUTABLE)_update.exe Makefile res src levels *.dll README.md
	upload $(EXECUTABLE).zip
zipall: all
	zip -r -u $(EXECUTABLE)_all.zip * -x *.zip
	zip -u $(EXECUTABLE)_all.zip sdl.zip
	upload $(EXECUTABLE)_all.zip


F=level
asm:
	$(CC) $(CFLAGS) -o $(EXECUTABLE).asm -S src/$(F).cpp $(LIBS) -I$(WINFOLDER)include/SDL2 -Dmain=SDL_main $(EXTRACFLAGS) -Iexternals/include
	meld $(EXECUTABLE).asm $(EXECUTABLE)_2.asm
asm2:
	$(CC) $(CFLAGS) -o $(EXECUTABLE)_2.asm -S src/$(F).cpp $(LIBS) -I$(WINFOLDER)include/SDL2 -Dmain=SDL_main $(EXTRACFLAGS) -Iexternals/include
asm3:
	$(CC) $(CFLAGS) -o $(EXECUTABLE)_3.asm -S src/$(F).cpp $(LIBS)
	meld $(EXECUTABLE).asm $(EXECUTABLE)_2.asm $(EXECTUABLE)_3.asm
