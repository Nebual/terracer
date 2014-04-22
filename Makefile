EXECUTABLE=terracer
EXTRALIBS=-lSDL2_image -lm -lSDL2_ttf -lSDL2_mixer

#SOURCES=src/main.c $(wildcard src/*.c)
SOURCES=entity.cpp input.cpp main.cpp level.cpp util.cpp
EXTERNALSOURCES=jsoncpp.cpp

CC=g++
C99MODE=-std=c++0x
EXTRACFLAGS=-g $(C99MODE) -Wuninitialized -Wmissing-field-initializers

debug: EXTRACFLAGS +=-DDEBUG -g
warn: EXTRACFLAGS += -Wall -Wextra

# Targetting Windows x64
win32: CC:=x86_64-w64-mingw32-$(CC)
win32: EXECUTABLE:=$(EXECUTABLE).exe
win32: CFLAGS=-I$(WINFOLDER)include/SDL2 -Dmain=SDL_main $(EXTRACFLAGS) -Iexternals/include
win32: LIBS=-L$(WINFOLDER)lib -lmingw32 -lSDL2main -lSDL2 $(EXTRALIBS)


ifeq ($(OS),Windows_NT)
# Building on Windows
WINFOLDER:=C:/mingw64/x86_64-w64-mingw32/

all: win32
win32: compile
run: win32
	@echo
	./$(EXECUTABLE)
else
# Building on Linux
WINFOLDER:=/usr/x86_64-w64-mingw32/
CFLAGS=$(shell sdl2-config --cflags) $(EXTRACFLAGS) -Iexternals/include
LIBS=$(shell sdl2-config --libs) $(EXTRALIBS)

all: compile
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
	$(CC) $(CFLAGS) -o $@ -c $<
$(EXTERNALOBJECTS): externals/build/%.o : externals/src/%.cpp
	$(CC) -Iexternals/include -o $@ -c $<
clean:
	-rm build/*.o
clean2: #A second one so we can execute clean twice
	-rm build/*.o
cleanall:
	-rm build/*.o
	-rm externals/build/*.o

compile: $(OBJECTS) $(EXTERNALOBJECTS)
	@echo
	@echo Linking...
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(EXTERNALOBJECTS) $(LIBS)


zip: win32
	zip -r -u $(EXECUTABLE).zip $(EXECUTABLE).exe $(EXECUTABLE).bat $(EXECUTABLE)_update.exe Makefile res src levels *.dll README.md
	upload $(EXECUTABLE).zip
zipall: all
	zip -r -u $(EXECUTABLE)_all.zip * -x *.zip
	zip -u $(EXECUTABLE)_all.zip sdl.zip
	upload $(EXECUTABLE)_all.zip


F=level
asm:
	$(CC) $(CFLAGS) -o $(EXECUTABLE).asm -S src/$(F).cpp $(LIBS)
	meld $(EXECUTABLE).asm $(EXECUTABLE)_2.asm
asm2:
	$(CC) $(CFLAGS) -o $(EXECUTABLE)_2.asm -S src/$(F).cpp $(LIBS)
asm3:
	$(CC) $(CFLAGS) -o $(EXECUTABLE)_3.asm -S src/$(F).cpp $(LIBS)
	meld $(EXECUTABLE).asm $(EXECUTABLE)_2.asm $(EXECTUABLE)_3.asm
