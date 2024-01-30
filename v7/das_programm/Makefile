CXXFLAGS_DEBUG = -std=c++20 -Wextra -Wall -g

default: build_dir
	g++ $(CXXFLAGS_DEBUG) src/send.cpp src/receive.cpp src/test.cpp src/common.cpp src/pack.cpp -o build/test

build_dir:
	mkdir -p build

build_tests:
	g++ $(CXXFLAGS_DEBUG) src/common.cpp src/pack.cpp src/doctest_main.cpp src/receive.cpp src/send.cpp -o build/tests

main:
	g++ $(CXXFLAGS_DEBUG) src/send.cpp src/receive.cpp src/main.cpp src/common.cpp src/pack.cpp -lb15fdrv -o build/main

run:
	./build/test

test:
	./build/tests

