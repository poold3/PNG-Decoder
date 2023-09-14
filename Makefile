H_FILES = $(wildcard headers/*.h)
SRC_FILES = $(wildcard src/*.cpp)

main: $(H_FILES) $(SRC_FILES) Makefile
	g++ -Wall -Wextra -Werror -Wno-error=unused-parameter -Wno-error=unused-variable -std=c++17 -I/home/poold/PNG-Data/headers $(SRC_FILES) -o main -lz