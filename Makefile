CC = gcc
OFLAGS = -g -O
CFLAGS = $(OFLAGS) \
		 -std=c2x \
		 -Wextra -Wall \
		 -Wno-unused-parameter -Wno-missing-field-initializers \
		 -fsanitize=undefined -fsanitize=address \
		 -pthread
		 -D_POSIX_C_SOURCE=199309L

INCLUDES = -I./src \
		   -I./libs/sokol \
		   -I./libs/cglm/include
LIBS = -lGL -ldl -lm -lX11 -lasound -lXi -lXcursor


SRC = $(shell find ./src -type f -name "*.c")
HDR = $(shell find ./src -type f -name "*.h")
OBJ = $(SRC:.c=.o)

SHDC = ./libs/sokol-tools-bin/bin/linux/sokol-shdc
SHDFLAGS = -l glsl430

SHD_SRC = $(shell find ./src -type f -name "*.glsl")
SHD_HDR = $(SHD_SRC:.glsl=.glsl.h)

all: clean shader compile

%.o: %.c $(HDR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

compile: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(LIBS) -o $@ 

%.glsl.h: %.glsl
	$(SHDC) -i $< -o $@ $(SHDFLAGS)

shader: $(SHD_HDR)

run: shader compile
	./compile

clean:
	rm -f $(OBJ) compile

format: $(SRC) $(HDR)
	clang-format -i $(SRC) $(HDR)

.PHONY: clean
