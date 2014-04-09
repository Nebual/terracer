EXECUTABLE=terracer
EXTRAHEADERS=
EXTRALIBS=-lSDL2_image -lm -lSDL2_ttf -lSDL2_mixer

#SOURCES=src/main.c $(wildcard src/*.c)
#SOURCES=src/entity.c src/input.c src/main.c src/system.c src/util.c src/hud.c
SOURCES=src/entity.cpp src/input.cpp src/main.cpp src/level.cpp src/util.cpp

F=main
CC=g++
C99MODE=-std=c++0x

debug: EXTRACFLAGS +=-DDEBUG -g
warn: EXTRACFLAGS += -Wall

# Targetting Windows x64
win32: CC:=x86_64-w64-mingw32-$(CC)
win32: EXECUTABLE:=$(EXECUTABLE).exe
win32: CFLAGS=-I$(WINFOLDER)include/SDL2 -Dmain=SDL_main $(C99MODE) $(EXTRACFLAGS)
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
CFLAGS=$(shell sdl2-config --cflags) $(C99MODE) $(EXTRACFLAGS)
LIBS=$(shell sdl2-config --libs) $(EXTRALIBS)

all: compile
win32: all
	zip $(EXECUTABLE).zip $(EXECUTABLE)
	upload $(EXECUTABLE).zip
run: all
	@echo
	./$(EXECUTABLE) --software
endif


debug: run
warn: all

zip: win32
	zip -r -u $(EXECUTABLE).zip $(EXECUTABLE).exe $(EXECUTABLE).bat $(EXECUTABLE)_update.exe Makefile res src levels *.dll README.md
	upload $(EXECUTABLE).zip
zipall: all
	zip -r -u $(EXECUTABLE)_all.zip * -x *.zip
	zip -u $(EXECUTABLE)_all.zip sdl.zip
	upload $(EXECUTABLE)_all.zip

compile: 
	@echo
	$(CC) -g $(CFLAGS) -o $(EXECUTABLE) $(SOURCES) $(LIBS)
headers:
	makeheaders $(EXTRAHEADERS) $(SOURCES)

asm:
	$(CC) $(CFLAGS) -o $(EXECUTABLE).asm -S src/$(F).c $(LIBS)
	meld $(EXECUTABLE).asm $(EXECUTABLE)_2.asm
asm2:
	$(CC) $(CFLAGS) -o $(EXECUTABLE)_2.asm -S src/$(F).c $(LIBS)
asm3:
	$(CC) $(CFLAGS) -o $(EXECUTABLE)_3.asm -S src/$(F).c $(LIBS)
	meld $(EXECUTABLE).asm $(EXECUTABLE)_2.asm $(EXECTUABLE)_3.asm
