CXXFLAGS_DEBUG = -std=c++20 -Wextra -Wall -g -fsanitize=address

default: build_dir
	g++ $(CXXFLAGS_DEBUG) src/send.cpp src/receive.cpp src/test.cpp -o build/test src/common.cpp

build_dir:
	mkdir -p build

run:
	./build/test

