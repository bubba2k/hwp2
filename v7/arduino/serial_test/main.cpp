#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>

int main(int argc, char *argv[]) {

    if(argc != 3) {
        std::fprintf(stderr, "Usage %s: <serial> <serial>\n", argv[0]);
        std::exit(1);
    }
    std::fstream in(argv[1]);
    std::fstream out(argv[2]);


    unsigned char i = 0x0;
    unsigned char received = 0x0;
    while(true) {
        in << i++;
        out << i++;

        out >> received;
        std::cerr << "\"" << std::to_string(received) << "\"" << std::endl;
    }

    return 0;
};
