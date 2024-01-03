# Temporary makefile since I can't seem to get Raylib's official makefile to work properly.
# WARNING: DOES NOT WORK WHEN USING MINGW ON WINDOWS OR WHEN USING CLANG/TCC

CC = gcc
ifeq (${OS}, Windows_NT)
	/ = \\#
	RM = del
	OS_ARGS = -lraylib-WINDOWS -lopengl32 -lgdi32 -lwinmm -pthread
	OUTPUT = RaylibSortingVisualizer.exe
	F =
	DEBUG_DELETE =
else
	/ = /
	RM = rm
	OS_ARGS = -lraylib-MACOS -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL
	OUTPUT = RaylibSortingVisualizer
	F = -f
	DEBUG_DELETE = rm -rf RaylibSortingVisualizer.dSYM
endif
SOURCE = src$/main.c
GENERIC_COMMAND = ${CC} ${SOURCE} -o ${OUTPUT} -Iinclude -Llib ${OS_ARGS}

prod:
	${GENERIC_COMMAND} -O2
all:
	prod
debug:
	${GENERIC_COMMAND} -g
clean:
	${RM} ${F} ${OUTPUT}
	${DEBUG_DELETE}
