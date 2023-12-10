CXXFLAGS_DEBUG = -std=c++20 -Wextra -Wall -g -fsanitize=address

default: build_dir
	g++ $(CXXFLAGS_DEBUG) src/send.cpp src/receive.cpp src/test.cpp -o build/test src/common.cpp src/pack.cpp

build_dir:
	mkdir -p build

build_tests:
	g++ $(CXXFLAGS_DEBUG) src/common.cpp src/pack.cpp src/doctest_main.cpp -o build/tests

run:
	./build/test

test:
	./build/tests

